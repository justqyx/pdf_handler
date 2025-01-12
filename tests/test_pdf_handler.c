#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/pdf_handler.h"

// 辅助函数：读取文件内容
static unsigned char* read_file(const char* filename, size_t* size) {
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    rewind(file);

    unsigned char* buffer = (unsigned char*)malloc(*size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    if (fread(buffer, 1, *size, file) != *size) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return buffer;
}

// 测试用例：替换简单的文本
void test_simple_replacement() {
    const char* input_file = "tests/test.pdf";  // 使用一个包含 "test" 文本的测试 PDF 文件
    const char* target = "test";
    const char* replacement = "sample";
    size_t input_size;

    // 读取测试 PDF 文件
    unsigned char* input_data = read_file(input_file, &input_size);
    assert(input_data != NULL);

    size_t modified_size;
    unsigned char* result = replace_text_in_pdf_stream(
        input_data,
        input_size,
        target,
        replacement,
        &modified_size
    );

    assert(result != NULL);
    free(input_data);
    free(result);
    printf("Simple replacement test passed.\n");
}

// 测试用例：替换不存在的文本
void test_non_existent_text() {
    const char* input_file = "tests/test.pdf";
    const char* target = "nonexistent";
    const char* replacement = "replacement";
    size_t input_size;

    // 读取测试 PDF 文件
    unsigned char* input_data = read_file(input_file, &input_size);
    assert(input_data != NULL);

    size_t modified_size;
    unsigned char* result = replace_text_in_pdf_stream(
        input_data,
        input_size,
        target,
        replacement,
        &modified_size
    );

    assert(result == NULL);  // 应该返回 NULL，因为文本不存在
    free(input_data);
    printf("Non-existent text test passed.\n");
}

// 测试用例：替换为更长的文本
void test_longer_replacement() {
    const char* input_file = "tests/test.pdf";
    const char* target = "test";
    const char* replacement = "very long replacement";
    size_t input_size;

    // 读取测试 PDF 文件
    unsigned char* input_data = read_file(input_file, &input_size);
    assert(input_data != NULL);

    size_t modified_size;
    unsigned char* result = replace_text_in_pdf_stream(
        input_data,
        input_size,
        target,
        replacement,
        &modified_size
    );

    assert(result != NULL);
    free(input_data);
    free(result);
    printf("Longer replacement test passed.\n");
}

// 测试用例：替换为更短的文本
void test_shorter_replacement() {
    const char* input_file = "tests/test.pdf";
    const char* target = "test";
    const char* replacement = "t";
    size_t input_size;

    // 读取测试 PDF 文件
    unsigned char* input_data = read_file(input_file, &input_size);
    assert(input_data != NULL);

    size_t modified_size;
    unsigned char* result = replace_text_in_pdf_stream(
        input_data,
        input_size,
        target,
        replacement,
        &modified_size
    );

    assert(result != NULL);
    free(input_data);
    free(result);
    printf("Shorter replacement test passed.\n");
}

int main() {
    test_simple_replacement();
    test_non_existent_text();
    test_longer_replacement();
    test_shorter_replacement();
    printf("All tests passed!\n");
    return 0;
}
