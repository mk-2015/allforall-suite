# **be & bce**

Line spacing for this document is configured to 1.15 to ensure clarity and professional presentation across desktop and mobile devices.

## **Overview**

**be** (Binary Entropy) and **bce** (Binary Create Entropy) are lightweight C++ command-line tools designed for analyzing and generating binary entropy for compression, cryptographic, and random payload testing.

## **Features**

> * **be (Binary Entropy):** Calculates Shannon entropy (0.0 to 8.0 bits/byte) on binary assets. Supports percentage scaling (-p) and chunked block-by-block analysis (-b).  
> * **bce (Binary Create Entropy):** Generates binary assets with configurable file sizes (-s) and targeted entropy weighting (-e).

## **Technical Specifications**

| Feature | Specification   |
| :---- | :---- |
| Language Standard | C++17 or later |
| Supported Operating Systems | Windows, Linux / POSIX systems |
| Header Dependencies | <iostream>, <fstream>, <string>, <vector>, <cmath>, <random>, <iomanip>, <filesystem> |
| Exit Status Code | 0 \= Success, 1 \= File Error / Invalid Parameter |

## **Command Line Usage & Options Reference**

### **be (Binary Entropy) Syntax**

`be <filename> [OPTIONS]`

| Flag | Description | Behavior Details   |
| :---- | :---- | :---- |
| <filename> | Input Binary File | Path to target file. |
| -p, --percentage | Percentage Scale | Renders output scaled from 0.0% to 100.0%. |
| -b, --block-size <bytes> | Block Interval | Calculates Shannon entropy in chunked intervals (default: 1024). |
| -h, --help | Help Menu | Displays usage menu and exits. |

### **bce (Binary Create Entropy) Syntax**

`bce <output_file> [OPTIONS]`

| Flag | Description | Behavior Details   |
| :---- | :---- | :---- |
| <output\_file> | Output Binary File | Destination path for generated file. |
| -s, --size <bytes> | Payload Size | Target file size in bytes (default: 1024). |
| -e, --entropy <0.0-8.0> | Entropy Scale | Target entropy distribution factor from 0.0 to 8.0. |
| -h, --help | Help Menu | Displays usage menu and exits. |

