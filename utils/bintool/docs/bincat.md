# **bincat**

## **Overview**

**bincat** is a specialized binary stream viewer and concatenator designed to safely output binary data without shell corruption. It preserves printable ASCII characters and newline boundaries while converting non-printable bytes into ANSI-highlighted, bracketed hex (`<0x1e>`) or binary (`<0b00011110>`) escape tags. In text-only mode (`-t`), it filters out non-printable bytes entirely to extract continuous string sequences.

## **Features**

> * **Literal ASCII Preservation:** Outputs printable ASCII (`0x20`–`0x7E`) and newlines (`0x0A`) cleanly.
> * **Control Tag Escaping:** Wraps non-printable bytes in hex-bracketed (`<0xXX>`) or bit-bracketed (`<0bXXXXXXXX>`) representations.
> * **ANSI Tag Highlighting:** Colorizes generated tags in blue so synthesized byte tags are visually distinct from literal text present in the payload.
> * **Text-Only Stream Filtering:** Offers a `-t` flag to silently strip non-printable binary headers/padding and extract raw text payloads directly.
> * **Terminal Safe:** Prevents raw binary streams from triggering escape sequences or corrupting shell layout.

## **Technical Specifications**

| Feature | Specification |
| :---- | :---- |
| Language Standard | C++17 or later |
| Supported Operating Systems | Windows, Linux / POSIX systems |
| Header Dependencies | <iostream>, <fstream>, <string>, <vector>, <cctype>, <iomanip>, <bitset>, <filesystem> |
| Exit Status Code | 0 = Success, 1 = Error / File I/O Failure |

## **Command Line Usage & Options Reference**

### **Syntax**

bincat [input_files...] [OPTIONS]

### **Options Reference**

| Flag | Description | Behavior Details |
| :---- | :---- | :---- |
| [input_files...] | Input Sources | Paths to target binary files. Reads stdin if omitted. |
| -o, --output <file> | Output Destination | Directs output stream to a specific file target instead of stdout. |
| -b, --binary | Binary Tag Mode | Formats escape tags as 8-bit binary strings (e.g., `<0b00011110>`). |
| -u, --uppercase | Uppercase Hex Tags | Formats hex control brackets in uppercase (e.g., `<0x1E>`). |
| -C, --no-color | Disable ANSI Color | Suppresses ANSI color sequences for generated tags. |
| -t, --text-only | Text-Only Filtering | Suppresses non-printable control tags; outputs printable ASCII and \n only. |
| -h, --help | Help Menu | Displays usage parameters and exits. |

## **Generated Output Examples**

### **Standard Escaped Output (Hex)**

Command: `bincat header.bin`

Input Payload: `a` `0x1E` `b` `\n`
Output: `a\033[34m<0x1e>\033[0mb\n`

### **Binary Tag Mode (-b)**

Command: `bincat header.bin -b`

Input Payload: `a` `0x1E` `b` `\n`
Output: `a\033[34m<0b00011110>\033[0mb\n`

### **Text-Only Mode (-t)**

Command: `bincat binary_blob.elf -t`

Input Payload: `ELF` `0x02` `0x01` `\n` `hello`
Output: `ELF\nhello`