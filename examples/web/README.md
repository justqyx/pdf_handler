# PDF Handler Web Example

This example demonstrates how to use the PDF Handler WebAssembly module in a web browser.

## Features

- Upload PDF files
- Replace text in PDF documents
- Download processed PDFs
- Modern and responsive UI
- Real-time status updates

## Running the Example

1. Make sure you have Python 3 installed
2. Navigate to this directory
3. Run the server:
   ```bash
   python3 server.py
   ```
4. Open your browser and navigate to http://localhost:8000/examples/web/

## Important Notes

- The server script includes necessary CORS headers for WebAssembly to work properly
- Make sure the WebAssembly module (`pdf_handler.js` and `pdf_handler.wasm`) is built before running the example
- The example assumes the WebAssembly files are in the `wasm` directory at the project root

## Browser Requirements

- Modern browser with WebAssembly support
- JavaScript enabled
- File API support

## Security Considerations

- The example runs entirely in the browser - no files are uploaded to any server
- PDF processing is done locally using WebAssembly
- Original and processed files never leave your computer
