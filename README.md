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

## Dependencies

### PDFium
This project requires PDFium, Google's PDF rendering engine. 

#### macOS Installation
```bash
brew install pdfium
```

The project expects PDFium to be installed in `/usr/local/opt/pdfium/`. If your installation is in a different location, update the paths in the Makefile.

## Building

1. Make sure you have PDFium installed
2. Clone this repository
3. Build the project:
```bash
make clean all
```

## Usage

```bash
./bin/pdf_handler <input_pdf> <output_pdf> <target_text> <replacement_text>
```

### Example
```bash
./bin/pdf_handler input.pdf output.pdf "old text" "new text"
```

## Testing

Run the test suite with:
```bash
make test
```

## Project Structure

```
.
├── src/                # Source files
│   ├── main.c         # Main program entry
│   └── pdf_handler.c  # Core PDF handling functionality
├── include/           # Header files
│   └── pdf_handler.h # Main header file
├── tests/            # Test files
├── lib/              # Libraries
└── obj/              # Object files (generated)
```

## Error Handling

The library includes comprehensive error handling. Check the return value and use `get_last_error()` for detailed error information.

## License

This project is licensed under the GNU General Public License v3 (GPLv3) - see the [LICENSE](LICENSE) file for details.

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Limitations

- Maximum supported PDF file size: 10MB
- Text replacement maintains original formatting but may not handle complex layouts
- Currently only supports text replacement (no image or other content types)

## Author

[Your Name]

## Acknowledgments

- PDFium team for the excellent PDF rendering engine
- All contributors who have helped with testing and improvements
