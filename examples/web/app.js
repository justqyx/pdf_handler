document.addEventListener('DOMContentLoaded', () => {
    const pdfInput = document.getElementById('pdfFile');
    const fileName = document.getElementById('fileName');
    const searchText = document.getElementById('searchText');
    const replaceText = document.getElementById('replaceText');
    const processButton = document.getElementById('processButton');
    const status = document.getElementById('status');
    const downloadLink = document.getElementById('downloadLink');
    
    let pdfData = null;
    let wasmModule = null;
    let replaceTextInPdf = null;

    // 错误代码映射
    const ERROR_CODES = {
        0: 'Invalid parameters',
        '-1': 'Memory error',
        '-2': 'Failed to load document',
        '-3': 'Failed to save document',
        '-4': 'Text not found in document',
    };

    // 初始化 WebAssembly 模块
    Module.onRuntimeInitialized = () => {
        wasmModule = Module;
        console.log('Available functions:', Object.keys(wasmModule));
        
        try {
            replaceTextInPdf = wasmModule.ccall.bind(wasmModule);
            status.textContent = 'WebAssembly module loaded successfully';
        } catch (error) {
            console.error('Error initializing function:', error);
            status.textContent = 'Error initializing WebAssembly module';
            status.classList.add('error');
        }
    };

    // 处理文件选择
    pdfInput.addEventListener('change', (event) => {
        const file = event.target.files[0];
        if (file) {
            fileName.textContent = file.name;
            const reader = new FileReader();
            
            reader.onload = (e) => {
                pdfData = new Uint8Array(e.target.result);
                console.log('PDF loaded, size:', pdfData.length, 'bytes');
                console.log('PDF header:', Array.from(pdfData.slice(0, 4)).map(b => String.fromCharCode(b)).join(''));
                processButton.disabled = !(pdfData && searchText.value && replaceText.value && replaceTextInPdf);
            };
            
            reader.readAsArrayBuffer(file);
        }
    });

    // 根据输入启用/禁用处理按钮
    [searchText, replaceText].forEach(input => {
        input.addEventListener('input', () => {
            processButton.disabled = !(pdfData && searchText.value && replaceText.value && replaceTextInPdf);
        });
    });

    // 处理 PDF
    processButton.addEventListener('click', async () => {
        if (!pdfData || !wasmModule || !replaceTextInPdf) return;

        let inputPtr = null;
        let sizePtr = null;
        try {
            status.textContent = 'Processing PDF...';
            processButton.disabled = true;
            downloadLink.style.display = 'none';
            status.classList.remove('error');

            // 验证 PDF 格式
            if (pdfData.length < 4 || 
                String.fromCharCode(pdfData[0]) !== '%' || 
                String.fromCharCode(pdfData[1]) !== 'P' || 
                String.fromCharCode(pdfData[2]) !== 'D' || 
                String.fromCharCode(pdfData[3]) !== 'F') {
                throw new Error('Invalid PDF format');
            }

            console.log('Allocating memory for input buffer and size...');
            inputPtr = wasmModule._malloc(pdfData.length);
            sizePtr = wasmModule._malloc(8); // 为 size_t 分配 8 字节
            if (!inputPtr || !sizePtr) {
                throw new Error('Failed to allocate memory');
            }
            console.log('Memory allocated at addresses:', 
                      '\n - Input buffer:', inputPtr,
                      '\n - Size pointer:', sizePtr);
            
            console.log('Copying PDF data to WebAssembly memory...');
            wasmModule.HEAPU8.set(pdfData, inputPtr);

            console.log('Calling replace_text_in_pdf_stream with parameters:',
                      '\n - Input buffer:', inputPtr,
                      '\n - Buffer size:', pdfData.length,
                      '\n - Search text:', searchText.value,
                      '\n - Replace text:', replaceText.value,
                      '\n - Size pointer:', sizePtr);

            const result = replaceTextInPdf(
                'replace_text_in_pdf_stream',
                'number',
                ['number', 'number', 'string', 'string', 'number'],
                [inputPtr, pdfData.length, searchText.value, replaceText.value, sizePtr]
            );

            console.log('Function returned:', result);

            if (result <= 0) {
                const errorMessage = ERROR_CODES[result] || 'Unknown error';
                throw new Error('Failed to process PDF: ' + errorMessage + ' (code: ' + result + ')');
            }

            // 获取处理后的 PDF 数据
            const modifiedSize = wasmModule.HEAPU32[sizePtr >> 2]; // 读取 size_t 值
            console.log('Modified PDF size:', modifiedSize, 'bytes');
            const processedData = wasmModule.HEAPU8.slice(result, result + modifiedSize);
            
            // 验证处理后的数据
            if (processedData.length === 0 || 
                String.fromCharCode(processedData[0]) !== '%' || 
                String.fromCharCode(processedData[1]) !== 'P' || 
                String.fromCharCode(processedData[2]) !== 'D' || 
                String.fromCharCode(processedData[3]) !== 'F') {
                throw new Error('Invalid PDF data produced');
            }
            
            console.log('Creating PDF blob...');
            const blob = new Blob([processedData], { type: 'application/pdf' });

            // 创建下载链接
            console.log('Creating download link...');
            const url = URL.createObjectURL(blob);
            downloadLink.href = url;
            downloadLink.download = 'processed_' + fileName.textContent;
            downloadLink.style.display = 'inline-block';
            status.textContent = 'PDF processed successfully!';
        } catch (error) {
            console.error('Error details:', error);
            status.textContent = 'Error: ' + error.message;
            status.classList.add('error');
            downloadLink.style.display = 'none';
        } finally {
            // 清理
            if (inputPtr !== null) {
                console.log('Freeing input buffer memory...');
                wasmModule._free(inputPtr);
            }
            if (sizePtr !== null) {
                console.log('Freeing size pointer memory...');
                wasmModule._free(sizePtr);
            }
            processButton.disabled = false;
        }
    });
});
