# PDF Handler

A C library for handling PDF files, with a focus on text replacement functionality. Built on top of PDFium, this tool allows you to replace text in PDF files while maintaining the original formatting, including font size and color.

## Features

- Replace text in PDF files while preserving:
  - Original font size
  - Original text color
  - Original text position
- Handles various text replacement scenarios:
  - Simple text replacement
  - Longer replacement text
  - Shorter replacement text
- Comprehensive error handling
- Memory-safe operations
- Support for both native and WebAssembly builds

## Dependencies

### PDFium
This project uses PDFium, Google's PDF rendering engine. 

#### For Native Build
The project includes a precompiled PDFium library in the `lib/pdfium` directory.

#### For WebAssembly Build
The WebAssembly version of PDFium is also included in the project.

### Emscripten (for WebAssembly build)
To build the WebAssembly version, you need Emscripten installed:
```bash
brew install emscripten
```

## Building

### Native Build
```bash
make clean all
```

### WebAssembly Build
```bash
make -f wasm.mk
```

## Usage

### Native Version
```bash
./bin/pdf_handler <input_pdf> <output_pdf> <target_text> <replacement_text>
```

Example:
```bash
./bin/pdf_handler input.pdf output.pdf "old text" "new text"
```

### WebAssembly Version
The WebAssembly build provides these functions:
```javascript
// Replace text in PDF
Module.ccall('replace_text_in_pdf_stream', 
            'number',                      // Return type
            ['array', 'number', 'string', 'string'], // Argument types
            [pdfData, pdfSize, targetText, replacementText]); // Arguments

// Get last error message
Module.ccall('get_last_error',
            'string',  // Return type
            [],        // No arguments
            []);      // No arguments
```

## Project Structure

```
.
├── src/              # Source files
│   ├── main.c       # Main program entry
│   └── pdf_handler.c # Core PDF handling functionality
├── include/          # Header files
│   └── pdf_handler.h # Main header file
├── tests/           # Test files
├── lib/             # Libraries
│   └── pdfium/     # PDFium library files
├── build/           # Build artifacts
├── bin/            # Compiled binaries
└── wasm/           # WebAssembly output
```

## Testing

Run the test suite with:
```bash
make test
```

## Error Handling

The library includes comprehensive error handling. Check the return value and use `get_last_error()` for detailed error information.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
