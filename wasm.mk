# WebAssembly 编译配置
EMCC = emcc
EMCFLAGS = -O2 \
           -s WASM=1 \
           -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
           -s EXPORTED_FUNCTIONS='["_replace_text_in_pdf_stream", "_get_last_error", "_malloc", "_free"]' \
           -s ALLOW_MEMORY_GROWTH=1 \
           -s USE_PTHREADS=0 \
           -s ASSERTIONS=1 \
           -I./include \
           -I./lib/pdfium/include

# 输出目录
WASM_DIR = wasm

# 源文件
WASM_SOURCES = src/pdf_handler.c

# PDFium 静态库
PDFIUM_LIB = lib/pdfium/lib/libpdfium.a

# WebAssembly 构建目标
.PHONY: wasm clean-wasm

wasm: $(WASM_DIR)/pdf_handler.js

$(WASM_DIR)/pdf_handler.js: $(WASM_SOURCES) $(PDFIUM_LIB) | $(WASM_DIR)
	$(EMCC) $(EMCFLAGS) $(WASM_SOURCES) $(PDFIUM_LIB) -o $@

$(WASM_DIR):
	mkdir -p $@

clean-wasm:
	rm -rf $(WASM_DIR)
