# **bhxx**

## **Overview**

**bhxx** is an advanced, high-performance C++ command-line binary hex viewing and reversing utility. It provides feature parity with standard xxd viewing options while integrating seamlessly with the "All For All" toolkit (deferring C-array output to bin2cl). It introduces enhanced capabilities such as ANSI byte-type colorization, custom column grouping, seek offsets, bit dumps, autoskipping, and robust reverse patching.

## **Features**

> * **Custom Column & Grouping Control:** Configure custom byte alignments per line (-c) and custom word grouping sizes (-g).
> * **ANSI Colorization (-C, --color):** Visually categorizes bytes dynamically by type: Null bytes (dark gray), Printable ASCII (green), 0xFF markers (red), and non-printable control characters (cyan).
> * **Bit-Level Dumps (-b):** Formats binary stream output as raw 8-bit sequences (01000001).
> * **Autoskip Repeated Lines (-a):** Replaces consecutive duplicate binary chunks with a single * indicator to minimize output bloat.
> * **Endian & Casing Formatting (-e, -u):** Supports little-endian byte-swapped display modes and forced uppercase hexadecimal letters.
> * **Decimal Offsets (-d):** Displays stream offset counters in decimal format instead of default hexadecimal.
> * **Targeted Seeking & Bounded Dumps (-s, -l):** Jump directly to byte offsets and restrict total byte processing output.
> * **Stream & File Reversing (-r):** Safely converts formatted hex dump input streams back into raw binary files.
> * **Flexible Stream Redirection:** Defaults to stdin and stdout when positional arguments are omitted, enabling seamless pipeline integration.

## **Technical Specifications**

| Feature | Specification |
| :---- | :---- |
| Language Standard | C++17 or later |
| Supported Operating Systems | Windows, Linux / POSIX systems |
| Header Dependencies | <iostream>, <fstream>, <string>, <vector>, <iomanip>, <cctype>, <algorithm>, <filesystem>, <bitset> |
| Exit Status Code | 0 = Success, 1 = Error / File I/O Failure |

## **Command Line Usage & Options Reference**

### **Syntax**

```bash
bhxx [input_file] [output_file] [OPTIONS]
```

### **Options Reference**

command: `bhxx`

| Flag | Description | Behavior Details |
| :---- | :---- | :---- |
| [input_file] | Input Source (Optional) | Path to target file. Defaults to stdin if omitted. |
| [output_file] | Destination (Optional) | Output target path. Defaults to stdout if omitted. |
| -a | Autoskip Duplicates | Replaces identical consecutive lines with an asterisk *. |
| -b | Binary Bit Dump | Outputs 8-bit binary digits instead of hex octets (defaults to 6 columns). |
| -c, --cols <bytes> | Column Formatting | Formats specified number of bytes per output line (default: 16). |
| -d | Decimal Offsets | Renders line offset counter in decimal instead of standard hex. |
| -e | Little-Endian Mode | Dumps output in little-endian byte-swapped order (default grouping: 4). |
| -g, --group <bytes> | Byte Grouping | Groups output by byte chunks (default: 2, set to 0 to disable). |
| -l, --len <bytes> | Bounded Reading | Stops stream reading after processing specified byte length. |
| -p, --plain | Plain Stream Mode | Outputs continuous plain hexadecimal characters without offsets or ASCII sidebar. |
| -r, --reverse | Reverse Convert | Converts formatted hex dump input streams back into raw binary. |
| -s, --seek <offset> | Seek Offset | Skips ahead to specified byte offset before processing stream. |
| -u | Uppercase Hex | Uses uppercase hexadecimal digits (e.g., 0xFF). |
| -C, --color | ANSI Colorization | Applies color escape codes based on byte classifications. |
| -h, --help | Help Menu | Displays command line parameters and exits. |

## **Generated Output Examples**

### **Standard Hex Dump Output**

Command: `bhxx sample.bin`

```bash
00000000: 7f45 4c46 0201 0100 0000 0000 0000 0000  .ELF............
00000100: 0200 3e00 0100 0000 0010 0000 0000 0000  ..>.............
```

### **Binary Bit Dump Mode**

Command: `bhxx sample.bin -b`

```bash
00000000: 01111111 01000101 01001100 01000110 00000010 00000001  .ELF..
```