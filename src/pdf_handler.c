#include <fpdfview.h>
#include <fpdf_edit.h>
#include <fpdf_text.h>
#include <fpdf_save.h>
#include <fpdf_formfill.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// 错误码定义
typedef enum {
    PDF_SUCCESS = 0,
    PDF_ERROR_INVALID_PARAMS = -1,
    PDF_ERROR_LOAD_FAILED = -2,
    PDF_ERROR_NO_TEXT_FOUND = -3,
    PDF_ERROR_MEMORY_ERROR = -4,
    PDF_ERROR_SAVE_FAILED = -5
} PdfError;

// 错误信息结构体
typedef struct {
    PdfError error_code;
    char error_message[256];
} PdfErrorInfo;

// 全局错误信息
static PdfErrorInfo g_last_error = {PDF_SUCCESS, ""};

// 用于文件写入的全局变量
static FILE* g_output_file = NULL;

// 设置错误信息
static void set_error(PdfError code, const char* message) {
    g_last_error.error_code = code;
    strncpy(g_last_error.error_message, message, sizeof(g_last_error.error_message) - 1);
    g_last_error.error_message[sizeof(g_last_error.error_message) - 1] = '\0';
}

// 获取最后的错误信息
PdfErrorInfo get_last_error(void) {
    return g_last_error;
}

// 自定义写入函数
static int WriteBlockCallback(struct FPDF_FILEWRITE_* pThis, const void* data, unsigned long size) {
    if (g_output_file == NULL) return 0;
    return fwrite(data, 1, size, g_output_file) == size;
}

// 将UTF-16LE字符串转换为UTF-8
static char* utf16le_to_utf8(const unsigned short* utf16, int len) {
    if (!utf16 || len <= 0) return NULL;
    
    // 预估UTF-8需要的大小（每个UTF-16字符最多需要3个UTF-8字节）
    int utf8_size = len * 3 + 1;
    char* utf8 = (char*)malloc(utf8_size);
    if (!utf8) return NULL;
    
    int utf8_len = 0;
    for (int i = 0; i < len; i++) {
        unsigned short unicode = utf16[i];
        
        if (unicode < 0x80) {
            // ASCII字符
            if (utf8_len + 1 >= utf8_size) break;
            utf8[utf8_len++] = (char)unicode;
        } else if (unicode < 0x800) {
            // 2字节UTF-8
            if (utf8_len + 2 >= utf8_size) break;
            utf8[utf8_len++] = 0xC0 | (unicode >> 6);
            utf8[utf8_len++] = 0x80 | (unicode & 0x3F);
        } else {
            // 3字节UTF-8
            if (utf8_len + 3 >= utf8_size) break;
            utf8[utf8_len++] = 0xE0 | (unicode >> 12);
            utf8[utf8_len++] = 0x80 | ((unicode >> 6) & 0x3F);
            utf8[utf8_len++] = 0x80 | (unicode & 0x3F);
        }
    }
    utf8[utf8_len] = '\0';
    return utf8;
}

// 将UTF-8字符串转换为UTF-16LE
static unsigned short* utf8_to_utf16le(const char* utf8, int* out_len) {
    if (!utf8 || !out_len) return NULL;
    
    int utf8_len = strlen(utf8);
    // 预估UTF-16需要的大小（每个UTF-8字符最多需要1个UTF-16字符）
    int utf16_size = (utf8_len + 1) * sizeof(unsigned short);
    unsigned short* utf16 = (unsigned short*)malloc(utf16_size);
    if (!utf16) return NULL;
    
    int utf16_len = 0;
    int i = 0;
    while (i < utf8_len) {
        unsigned int unicode = 0;
        unsigned char c = utf8[i];
        
        if ((c & 0x80) == 0) {
            // ASCII字符
            unicode = c;
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            // 2字节UTF-8
            if (i + 1 >= utf8_len) break;
            unicode = ((c & 0x1F) << 6) | (utf8[i + 1] & 0x3F);
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3字节UTF-8
            if (i + 2 >= utf8_len) break;
            unicode = ((c & 0x0F) << 12) | 
                     ((utf8[i + 1] & 0x3F) << 6) |
                     (utf8[i + 2] & 0x3F);
            i += 3;
        } else {
            // 不支持的编码
            i += 1;
            continue;
        }
        
        if (utf16_len >= utf16_size / sizeof(unsigned short)) break;
        utf16[utf16_len++] = (unsigned short)unicode;
    }
    
    *out_len = utf16_len;
    return utf16;
}

unsigned char* replace_text_in_pdf_stream(
    const unsigned char* pdf_binary_stream,
    size_t pdf_stream_size,
    const char* target_text,
    const char* replacement_text,
    size_t* modified_pdf_size
) {
    if (pdf_binary_stream == NULL || pdf_stream_size == 0 || target_text == NULL || 
        replacement_text == NULL || modified_pdf_size == NULL) {
        set_error(PDF_ERROR_INVALID_PARAMS, "Invalid input parameters");
        return NULL;
    }

    // 重置错误状态
    set_error(PDF_SUCCESS, "");

    FPDF_LIBRARY_CONFIG config;
    config.version = 2;
    config.m_pUserFontPaths = NULL;
    config.m_pIsolate = NULL;
    config.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&config);

    FPDF_DOCUMENT doc = FPDF_LoadMemDocument(pdf_binary_stream, (int)pdf_stream_size, NULL);
    if (!doc) {
        set_error(PDF_ERROR_LOAD_FAILED, "Failed to load PDF document");
        FPDF_DestroyLibrary();
        return NULL;
    }

    int page_count = FPDF_GetPageCount(doc);
    if (page_count <= 0) {
        set_error(PDF_ERROR_LOAD_FAILED, "PDF document has no pages");
        FPDF_CloseDocument(doc);
        FPDF_DestroyLibrary();
        return NULL;
    }

    // 将替换文本转换为UTF-16LE
    int replacement_len = 0;
    unsigned short* replacement_utf16 = utf8_to_utf16le(replacement_text, &replacement_len);
    if (!replacement_utf16) {
        set_error(PDF_ERROR_MEMORY_ERROR, "Failed to convert replacement text to UTF-16");
        FPDF_CloseDocument(doc);
        FPDF_DestroyLibrary();
        return NULL;
    }

    int text_replaced = 0;
    for (int i = 0; i < page_count; i++) {
        FPDF_PAGE page = FPDF_LoadPage(doc, i);
        if (!page) {
            snprintf(g_last_error.error_message, sizeof(g_last_error.error_message),
                    "Failed to load page %d", i);
            continue;
        }

        FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);
        if (!text_page) {
            FPDF_ClosePage(page);
            continue;
        }

        // 获取页面上的所有对象
        int obj_count = FPDFPage_CountObjects(page);
        for (int obj_index = 0; obj_index < obj_count; obj_index++) {
            FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, obj_index);
            if (!obj) continue;

            // 检查是否是文本对象
            if (FPDFPageObj_GetType(obj) == FPDF_PAGEOBJ_TEXT) {
                unsigned short buffer[1024];  // 假设单个文本对象不会超过1024个字符
                unsigned long len = FPDFTextObj_GetText(obj, text_page, buffer, sizeof(buffer)/sizeof(buffer[0]));
                
                if (len > 0) {
                    // 将文本转换为UTF-8进行比较
                    char* obj_text = utf16le_to_utf8(buffer, len);
                    if (obj_text) {
                        if (strstr(obj_text, target_text) != NULL) {
                            // 获取对象的位置和属性
                            float left = 0, top = 0, right = 0, bottom = 0;
                            FPDFPageObj_GetBounds(obj, &left, &bottom, &right, &top);

                            // 获取字体大小和颜色
                            float font_size = 0;
                            if (!FPDFTextObj_GetFontSize(obj, &font_size)) {
                                font_size = 12.0f;  // 默认字体大小
                            }

                            unsigned int R = 0, G = 0, B = 0, A = 255;
                            int has_color = FPDFPageObj_GetFillColor(obj, &R, &G, &B, &A);

                            // 删除原始对象
                            FPDFPage_RemoveObject(page, obj);

                            // 创建新的文本对象
                            FPDF_PAGEOBJECT new_obj = FPDFPageObj_NewTextObj(doc, "Arial", font_size);
                            if (new_obj) {
                                // 设置文本内容
                                if (FPDFText_SetText(new_obj, replacement_utf16)) {
                                    // 计算垂直中心点，使用它作为基准点
                                    // float baseline = bottom + (top - bottom) * 0.025f;  // 降低基线位置
                                    FPDFPageObj_Transform(new_obj, 1.0, 0, 0, 1.0, left, bottom);

                                    // 设置颜色
                                    if (has_color) {
                                        FPDFPageObj_SetFillColor(new_obj, R, G, B, A);
                                    }

                                    // 添加到页面
                                    FPDFPage_InsertObject(page, new_obj);
                                    text_replaced = 1;
                                } else {
                                    FPDFPageObj_Destroy(new_obj);
                                }
                            }
                        }
                        free(obj_text);
                    }
                }
            }
        }

        // 生成页面内容
        if (text_replaced) {
            FPDFPage_GenerateContent(page);
        }

        FPDFText_ClosePage(text_page);
        FPDF_ClosePage(page);
    }

    free(replacement_utf16);

    if (!text_replaced) {
        set_error(PDF_ERROR_NO_TEXT_FOUND, "Target text not found in document");
        FPDF_CloseDocument(doc);
        FPDF_DestroyLibrary();
        return NULL;
    }

    FILE* temp_file = tmpfile();
    if (!temp_file) {
        set_error(PDF_ERROR_SAVE_FAILED, "Failed to create temporary file");
        FPDF_CloseDocument(doc);
        FPDF_DestroyLibrary();
        return NULL;
    }

    // 设置全局输出文件
    g_output_file = temp_file;

    FPDF_FILEWRITE file_write;
    file_write.version = 1;
    file_write.WriteBlock = WriteBlockCallback;

    if (!FPDF_SaveAsCopy(doc, &file_write, 0)) {
        set_error(PDF_ERROR_SAVE_FAILED, "Failed to save modified PDF");
        g_output_file = NULL;
        fclose(temp_file);
        FPDF_CloseDocument(doc);
        FPDF_DestroyLibrary();
        return NULL;
    }

    // 清除全局输出文件
    g_output_file = NULL;

    fseek(temp_file, 0, SEEK_END);
    *modified_pdf_size = ftell(temp_file);
    rewind(temp_file);

    unsigned char* result = (unsigned char*)malloc(*modified_pdf_size);
    if (!result) {
        set_error(PDF_ERROR_MEMORY_ERROR, "Failed to allocate memory for result");
        fclose(temp_file);
        FPDF_CloseDocument(doc);
        FPDF_DestroyLibrary();
        return NULL;
    }

    if (fread(result, 1, *modified_pdf_size, temp_file) != *modified_pdf_size) {
        set_error(PDF_ERROR_SAVE_FAILED, "Failed to read modified PDF");
        free(result);
        fclose(temp_file);
        FPDF_CloseDocument(doc);
        FPDF_DestroyLibrary();
        return NULL;
    }

    fclose(temp_file);
    FPDF_CloseDocument(doc);
    FPDF_DestroyLibrary();

    return result;
}
