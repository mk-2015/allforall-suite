# **filecmp**

Line spacing for this document is configured to 1.15 to ensure clarity and professional presentation across desktop and mobile devices.

## **Overview**

**filecmp** is a high-performance, cross-platform C++ command-line tool designed to compare two files based on various criteria, including raw binary/text contents, file sizes, ownership, short file names, and pattern occurrence distributions.

## **Features**

> * **Default Content Comparison:** Compares full file contents byte-by-byte or string-wise.  
> * **Size Comparison (-s):** Performs a fast file size equivalence check in bytes.  
> * **Ownership Comparison (-u):** Checks if both files share the same OS-level owner user account (supports Windows and Linux).  
> * **Short Name Comparison (-S):** Compares filenames regardless of parent directory paths (e.g., /dev/1 vs proc/1).  
> * **Pattern Match Comparison (-p \<pattern\>):** Verifies if a given pattern occurs at identical byte/character offset distributions in both files.  
> * **Case Sensitivity Control (-oC):** Enables case-sensitive processing for string and pattern comparisons (case-insensitive by default).  
> * **Silent & Exit-Code Driven:** Returns status code **0** on match and status code **1** on mismatch or execution error.

## **Technical Specifications**

| Feature | Specification   |
| :---- | :---- |
| Language Standard | C++17 or later |
| Supported Operating Systems | Windows, Linux / POSIX systems (excluding macOS) |
| Header Dependencies | \<iostream\>, \<fstream\>, \<string\>, \<vector\>, \<algorithm\>, \<cctype\>, \<filesystem\> |
| Platform APIs | windows.h (Windows), sys/stat.h / pwd.h (Linux) |
| Exit Status Code | 0 \= Match, 1 \= Mismatch / Error |

## **Command Line Usage & Options Reference**

Syntax:

`filecmp <file1> <file2> [-s] [-u] [-S] [-p <pattern>] [-oC]`

| Flag | Description | Behavior Details   |
| :---- | :---- | :---- |
| \<file1\> \<file2\> | Positional File Paths | Required positional arguments. Must point to valid files. |
| \-s | Size Comparison | Compares file sizes using std::filesystem::file\_size. |
| \-u | Owner Comparison | Compares owner usernames obtained from GetUserNameA (Windows) or getpwuid (Linux). |
| \-S | Short Name Comparison | Compares base filenames (e.g., dev/1 and proc/1 evaluate to equal '1'). |
| \-p \<pattern\> | Pattern Comparison | Finds all starting index positions of pattern in both files and verifies equality of position lists. |
| \-oC | Case Sensitivity | Enables strict case-sensitive comparisons for names, owners, and pattern matches. |

## **Pattern Match Comparison Logic**

Consider two files compared with \-p "hello":

> * **File 1:**  
>   hello world  
>   hello xword  
> * **File 2:**  
>   hello Banana  
>   bananaworld

Offset locations found in File 1 are \[0, 12\]. Offset locations found in File 2 are \[0\]. Because the vectors of offsets do not match, **filecmp** returns 1 (mismatch).