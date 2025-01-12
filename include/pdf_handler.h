#ifndef PDF_PROCESSOR_H
#define PDF_PROCESSOR_H

#include <stddef.h>

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