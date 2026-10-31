# **bin2cl**

## **Overview**

**bin2cl** is a lightweight C++ command-line utility that converts arbitrary binary files (such as images, shaders, audio files, or raw binary assets) into C or C++ header files containing const unsigned char arrays. This allows binary assets to be embedded directly into compiled executables without external runtime file dependencies.

## **Features**

> * **Dual Language Output:** Generates C++ headers (.hpp) by default using inline constexpr, or C headers (.h) via the -lc/--langc flag using static linkage.  
> * **Identifier Sanitization:** Automatically converts invalid C/C++ characters in filenames to underscores (e.g., logo.png becomes logo_png).  
> * **Include Guards:** Auto-generates uppercase include guards based on the output filename to prevent double-inclusion.  
> * **Size Constants:** Exports an accompanying _len constant tracking the precise byte length of the embedded buffer.  
> * **Exit-Code Driven:** Returns status code 0 on successful generation and status code 1 on missing files or invalid argument invocation.

## **Technical Specifications**

| Feature | Specification   |
| :---- | :---- |
| Language Standard | C++17 or later |
| Supported Operating Systems | Windows, Linux / POSIX systems |
| Header Dependencies | <iostream>, <fstream>, <string>, <vector>, <algorithm>, <cctype>, <iomanip>, <filesystem> |
| Exit Status Code | 0 = Success, 1 = Error |

## **Command Line Usage & Options Reference**

### **Syntax**

bin2cl <file> <file-to-output> [OPTIONS]

### **Options Reference**

| Flag | Description | Behavior Details   |
| :---- | :---- | :---- |
| <file> | Input Binary File | Required positional argument. Path to source binary asset. |
| <file-to-output> | Output Header Base Path | Required positional argument. Base output path (extension added automatically). |
| -lc, --langc | C Language Output | Generates a C header file (.h) instead of a C++ header (.hpp). |
| -h, --help | Help Menu | Displays usage parameters and exits. |

## **Generated Output Examples**

### **Default C++ Header**

Command: bin2cl logo.png asset_logo → asset_logo.hpp

```c
#ifndef ASSET_LOGO_HPP_INCLUDED  
#define ASSET_LOGO_HPP_INCLUDED

#include <cstddef>

inline constexpr unsigned char logo_png[] = {  
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d  
};  
inline constexpr size_t logo_png_len = 12;

#endif // ASSET_LOGO_HPP_INCLUDED

```

### **C Header Option**

Command: bin2cl logo.png asset_logo -lc → asset_logo.h

```c
#ifndef ASSET_LOGO_H_INCLUDED  
#define ASSET_LOGO_H_INCLUDED

#include <stddef.h>

static const unsigned char logo_png[] = {  
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d  
};  
static const size_t logo_png_len = 12;

#endif // ASSET_LOGO_H_INCLUDED

```