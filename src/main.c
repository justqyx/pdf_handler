#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/pdf_handler.h"

#define MAX_FILE_SIZE 10485760  // 10 MB

/**
 * 读取文件内容
 * 
 * 该函数将文件内容读取到内存中，并将文件大小写入 file_size 中。
 * 如果读取失败，会返回 NULL。
 * 
 * @param filename 文件名
 * @param file_size 文件大小（输出参数）
 * @return 文件内容
 */
unsigned char* read_file(const char* filename, size_t* file_size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    if (*file_size > MAX_FILE_SIZE) {
        fprintf(stderr, "File too large\n");
        fclose(file);
        return NULL;
    }

    unsigned char* buffer = (unsigned char*)malloc(*file_size);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(buffer, 1, *file_size, file);
    if (read_size != *file_size) {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return buffer;
}

/**
 * 将二进制数据写入到文件中
 * 
 * 该函数将 data 中的 size 字节写入到 filename 指定的文件中。
 * 如果写入成功，返回 1，否则返回 0。
 * 
 * @param filename 文件名
 * @param data 要写入的数据
 * @param size 数据大小
 * @return 写入是否成功
 */
int write_file(const char* filename, const unsigned char* data, size_t size) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file for writing");
        return 0;
    }

    size_t written = fwrite(data, 1, size, file);
    fclose(file);

    return (written == size);
}

/**
 * 主函数
 *
 * 该函数从命令行参数中获取四个参数：
 *  input_pdf:  输入 PDF 文件名
 *  output_pdf:  输出 PDF 文件名
 *  target_text:要在 PDF 中搜索和替换的文本
 *  replacement_text:将 target_text 替换为的新文本
 *
 * 该函数从输入 PDF 文件中读取内容，使用 replace_text_in_pdf_stream
 * 函数将 target_text 替换为 replacement_text，最后将结果写入到输出
 * PDF 文件中。
 *
 * @param argc  argc
 * @param argv  argv
 * @return      0 if successful, 1 if failed.
 */
int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <input_pdf> <output_pdf> <target_text> <replacement_text>\n", argv[0]);
        return 1;
    }

    const char* input_filename = argv[1];   // 输入 PDF 文件名
    const char* output_filename = argv[2];  // 输出 PDF 文件名
    const char* target_text = argv[3];      // 要在 PDF 中搜索和替换的文本
    const char* replacement_text = argv[4]; // 将 target_text 替换为的新文本

    // 读取输入 PDF 文件
    size_t pdf_size;
    unsigned char* pdf_content = read_file(input_filename, &pdf_size);
    if (!pdf_content) {
        return 1;
    }

    // 将目标文本替换为新文本
    size_t modified_size;
    unsigned char* result = replace_text_in_pdf_stream(
        pdf_content,
        pdf_size,
        target_text,
        replacement_text,
        &modified_size
    );

    // 释放原始 PDF 内容
    free(pdf_content);

    // 如果替换失败，返回错误
    if (result == NULL) {
        fprintf(stderr, "Failed to replace text in PDF.\n");
        return 1;
    }

    // 将结果写入到输出 PDF 文件中
    if (!write_file(output_filename, result, modified_size)) {
        fprintf(stderr, "Failed to write output file.\n");
        free(result);
        return 1;
    }

    // 释放结果内存
    free(result);

    printf("Text replacement completed. Output written to %s\n", output_filename);

    return 0;
}
