# PDF Handler

一个基于 PDFium 的 PDF 文本替换工具，支持原生编译和 WebAssembly。

## 功能特性

- 在 PDF 文件中搜索和替换文本
- 支持原生编译和 WebAssembly
- 保持原始 PDF 格式和布局
- 提供简单的 Web 界面示例

## 系统要求

- C 编译器（gcc 或 clang）
- PDFium 库
- Emscripten（用于 WebAssembly 编译）
- Python 3（用于运行示例服务器）

## 安装依赖

### 安装 Emscripten

1. 克隆 emsdk 仓库：
   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ```

2. 拉取最新版本：
   ```bash
   git pull
   ```

3. 下载并安装最新的 SDK 工具：
   ```bash
   ./emsdk install latest
   ```

4. 激活 SDK：
   ```bash
   ./emsdk activate latest
   ```

5. 设置环境变量：
   ```bash
   source ./emsdk_env.sh  # Unix/macOS
   # 或
   emsdk_env.bat  # Windows
   ```

6. 验证安装：
   ```bash
   emcc --version
   ```

### 安装 PDFium

PDFium 库已包含在项目中：
- 原生版本：`lib/pdfium/lib/libpdfium.a`
- WebAssembly 版本：`lib/pdfium/wasm/libpdfium.a`

如果需要自行编译 PDFium：
1. 克隆 PDFium 仓库
2. 按照 PDFium 的构建说明进行编译
3. 将编译好的库文件复制到相应目录

## 编译说明

### 原生编译

```bash
# 编译项目
make

# 运行测试
make test
```

### WebAssembly 编译

```bash
# 设置 Emscripten 环境（如果尚未设置）
source path/to/emsdk/emsdk_env.sh  # Unix/macOS
# 或
path\to\emsdk\emsdk_env.bat  # Windows

# 编译 WebAssembly 模块
make -f wasm.mk

# 清理 WebAssembly 构建
make -f wasm.mk clean-wasm
```

## 使用方法

### 命令行工具

```bash
# 替换 PDF 中的文本
./bin/pdf_handler input.pdf output.pdf "原文本" "新文本"
```

### Web 界面

1. 编译 WebAssembly 模块：
   ```bash
   make -f wasm.mk
   ```

2. 启动示例服务器：
   ```bash
   cd examples/web
   python3 server.py
   ```

3. 在浏览器中访问：`http://localhost:8000`

## API 说明

### C API

```c
// 替换 PDF 二进制流中的文本
unsigned char* replace_text_in_pdf_stream(
    const unsigned char* pdf_binary_stream,
    size_t pdf_stream_size,
    const char* target_text,
    const char* replacement_text,
    size_t* modified_pdf_size
);

// 获取最后一次错误的代码
pdf_error_code_t get_last_error(void);

// 获取最后一次错误的消息
const char* get_last_error_message(void);
```

### JavaScript API

```javascript
// 替换 PDF 中的文本
const result = Module.ccall(
    'replace_text_in_pdf_stream',
    'number',
    ['number', 'number', 'string', 'string', 'number'],
    [inputPtr, pdfSize, searchText, replaceText, sizePtr]
);
```

## 错误代码

- `PDF_SUCCESS` (1): 操作成功
- `PDF_ERROR_INVALID_PARAMS` (0): 无效参数
- `PDF_ERROR_MEMORY_ERROR` (-1): 内存错误
- `PDF_ERROR_LOAD_FAILED` (-2): 加载 PDF 失败
- `PDF_ERROR_SAVE_FAILED` (-3): 保存 PDF 失败
- `PDF_ERROR_NO_TEXT_FOUND` (-4): 未找到目标文本

## 示例代码

### C 示例

```c
#include <pdf_handler.h>

// 读取 PDF 文件
FILE* file = fopen("input.pdf", "rb");
fseek(file, 0, SEEK_END);
size_t size = ftell(file);
rewind(file);

unsigned char* buffer = malloc(size);
fread(buffer, 1, size, file);
fclose(file);

// 替换文本
size_t new_size;
unsigned char* result = replace_text_in_pdf_stream(
    buffer, size, "old text", "new text", &new_size
);

if (result) {
    // 保存修改后的 PDF
    file = fopen("output.pdf", "wb");
    fwrite(result, 1, new_size, file);
    fclose(file);
    free(result);
} else {
    // 处理错误
    pdf_error_code_t error = get_last_error();
    const char* message = get_last_error_message();
    printf("Error: %s (code: %d)\n", message, error);
}

free(buffer);
```

### JavaScript 示例

```javascript
// 分配内存
const inputPtr = Module._malloc(pdfData.length);
const sizePtr = Module._malloc(8);

// 复制数据到 WebAssembly 内存
Module.HEAPU8.set(pdfData, inputPtr);

// 替换文本
const result = Module.ccall(
    'replace_text_in_pdf_stream',
    'number',
    ['number', 'number', 'string', 'string', 'number'],
    [inputPtr, pdfData.length, 'old text', 'new text', sizePtr]
);

if (result > 0) {
    // 读取修改后的 PDF 大小
    const modifiedSize = Module.HEAPU32[sizePtr >> 2];
    
    // 获取处理后的数据
    const processedData = Module.HEAPU8.slice(result, result + modifiedSize);
    
    // 创建 Blob 并下载
    const blob = new Blob([processedData], { type: 'application/pdf' });
    const url = URL.createObjectURL(blob);
    // ... 处理下载
} else {
    console.error('Failed to process PDF');
}

// 清理内存
Module._free(inputPtr);
Module._free(sizePtr);
```

## 注意事项

1. 内存管理：
   - 在 C 代码中，使用返回的指针后必须释放内存
   - 在 JavaScript 代码中，必须释放通过 `Module._malloc` 分配的内存

2. 错误处理：
   - 总是检查返回值和错误代码
   - 使用 `get_last_error` 和 `get_last_error_message` 获取详细错误信息

3. PDF 格式：
   - 输入 PDF 必须是有效的 PDF 文件（以 %PDF 开头）
   - 某些加密或受保护的 PDF 可能无法处理

4. WebAssembly 相关：
   - 确保 Emscripten 环境正确设置
   - 编译时可能需要较大的内存（建议至少 4GB RAM）
   - 某些浏览器可能需要启用 WebAssembly 支持

## 故障排除

1. Emscripten 编译问题：
   - 确保 PATH 中包含 emcc
   - 检查 EMSDK 环境变量是否正确设置
   - 尝试重新激活 emsdk：`./emsdk activate latest`

2. 内存问题：
   - 如果遇到内存不足错误，可以增加 `-s TOTAL_MEMORY=xxxMB`
   - 对于大型 PDF，确保启用了内存增长：`-s ALLOW_MEMORY_GROWTH=1`

3. 浏览器兼容性：
   - 使用最新版本的现代浏览器
   - 确保浏览器支持 WebAssembly
   - 检查控制台是否有 CORS 相关错误

## 许可证

本项目采用 [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.html) 许可证。

这意味着您可以：
- 使用：出于任何目的运行此软件
- 研究：了解软件的工作原理并根据您的需要进行修改
- 分发：复制和分发软件
- 改进：改进软件并向公众发布改进的版本

但您必须：
- 包含原始源代码：任何分发都必须包含完整的源代码
- 标明更改：如果您修改了代码，必须明确标注您所做的更改
- 相同方式共享：任何修改和衍生作品必须以相同的许可证（GPLv3）发布
- 保留许可证和版权信息：您必须保留原始的许可证和版权信息

详细信息请参阅 [LICENSE](LICENSE) 文件。
