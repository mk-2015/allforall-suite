# **bpatch**

Line spacing for this document is configured to 1.15 to ensure clarity and professional presentation across desktop and mobile devices.

## **Overview**

**bpatch** is a targeted binary modification utility capable of overwriting or inserting raw hex sequences (\xHH), strings, or surrounded edit markers into a file at specific byte offsets without needing to re-encode or re-build the target asset.

## **Features**

> * **Targeted Offset Patching:** Modify binary files at exact byte offsets without re-encoding the entire asset.  
> * **Escaped Hex Parsing:** Parses standard escaped hexadecimal sequences (e.g., \x00\xAE\x41) or raw strings into binary payloads.  
> * **Insertion Mode (-i):** Allows inserting new byte sequences at a target offset rather than overwriting existing data.  
> * **Surrounded Edit Banners (-s):** Embeds clear boundary banners (--EDIT-> <hex> <-EDIT--) around patched sequences.  
> * **Dry Run Mode (-d):** Validates offset boundaries and hex formatting without writing modifications to the destination file.

## **Technical Specifications**

| Feature | Specification   |
| :---- | :---- |
| Language Standard | C++17 or later |
| Supported Operating Systems | Windows, Linux / POSIX systems |
| Header Dependencies | <iostream>, <fstream>, <string>, <vector>, <algorithm>, <sstream>, <iomanip>, <filesystem> |
| Exit Status Code | 0 \= Success, 1 \= Error / Invalid Offset |

## **Command Line Usage & Options Reference**

Syntax:

bpatch <file> <hex> [OPTIONS]

| Flag | Description | Behavior Details   |
| :---- | :---- | :---- |
| <file> | Target Binary File | Path to the binary file to patch. |
| <hex> | Escaped Hex Sequence | Escaped hex string (e.g., "\x00\xAE\x41"). |
| -o, --offset <bytes> | Target Offset | Byte location where patch is applied (default: 0). |
| -s, --surround | Marker Embedder | Encloses patch sequence inside --EDIT-> <hex> <-EDIT-- markers. |
| -i, --insert | Insertion Mode | Inserts bytes at target offset instead of overwriting existing bytes. |
| -d, --dry-run | Dry Run Simulation | Validates offset and hex payload without modifying target file. |
| -h, --help | Help Menu | Displays usage parameters and exits. |

