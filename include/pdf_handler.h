#ifndef PDF_PROCESSOR_H
#define PDF_PROCESSOR_H

#include <stddef.h>

// 错误代码定义
typedef enum {
    PDF_SUCCESS = 1,  // 改为 1，这样 0 就表示错误
    PDF_ERROR_INVALID_PARAMS = 0,
    PDF_ERROR_MEMORY_ERROR = -1,
    PDF_ERROR_LOAD_FAILED = -2,
    PDF_ERROR_SAVE_FAILED = -3,
    PDF_ERROR_NO_TEXT_FOUND = -4
} pdf_error_code_t;

/**
 * 获取最后一次错误的代码
 *
 * @return 错误代码
 */
pdf_error_code_t get_last_error(void);

/**
 * 获取最后一次错误的消息
 *
 * @return 错误消息字符串，如果没有错误则返回 NULL
 */
const char* get_last_error_message(void);

/**
 * 在 PDF 二进制流中替换文本
 *
 * 该函数将 target_text 替换为 replacement_text，并将结果写入到一个新的 PDF
 * 二进制流中。
 *
 * @param pdf_binary_stream  原始 PDF 二进制流
 * @param pdf_stream_size  原始流大小
 * @param target_text  要替换的目标文本
 * @param replacement_text  替换用的新文本
 * @param modified_pdf_size  修改后的 PDF 流大小（输出参数）
 * @return  修改后的 PDF 二进制流，如果失败则返回 NULL
 */
unsigned char* replace_text_in_pdf_stream(
    const unsigned char* pdf_binary_stream,
    size_t pdf_stream_size,
    const char* target_text,
    const char* replacement_text,
    size_t* modified_pdf_size
);

#endif // PDF_PROCESSOR_H