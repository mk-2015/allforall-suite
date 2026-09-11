# **hashbin**

Line spacing for this document is configured to 1.15 to ensure clarity and professional presentation across desktop and mobile devices.

## **Overview**

**hashbin** is a high-performance C++ command-line hashing tool designed to calculate cryptographic file hashes using OpenSSL. It supports multiple hashing algorithms along with custom salt and pepper input files to generate reproducible security digests.

## **Features**

> * **Multiple Algorithms:** Native support for sha256, sha512, sha1, md5, and argon2 (via OpenSSL 3.0 KDF).  
> * **Salt Support (-s):** Appends custom salt file contents to the input data prior to hashing.  
> * **Pepper Support (-p):** Appends custom pepper file contents to the input data prior to hashing.  
> * **Formatted Output:** Generates standardized digest strings in the format ALGONAME(FILENAME,SALTFILENAME,PEPPERFILENAME)=HASH.  
> * **Exit-Code Driven:** Returns status code **0** on success and status code **1** on missing files or execution errors.

## **Technical Specifications**

| Feature | Specification   |
| :---- | :---- |
| Language Standard | C++17 or later |
| Supported Operating Systems | Windows, Linux / POSIX systems (excluding macOS) |
| Header Dependencies | \<iostream\>, \<fstream\>, \<string\>, \<vector\>, \<algorithm\>, \<iomanip\>, \<sstream\>, \<filesystem\>, \<openssl/evp.h\>, \<openssl/kdf.h\> |
| Linking Dependencies | \-lcrypto (Linux/Windows), \-lws2\_32 \-ladvapi32 (Windows) |
| Exit Status Code | 0 \= Success, 1 \= Error |

## **Command Line Usage & Options Reference**

Syntax:

`hashbin <filename> [-a <algorithm>] [-s <saltfile>] [-p <pepperfile>]`

| Flag | Description | Behavior Details   |
| :---- | :---- | :---- |
| \<filename\> | Positional Target File | Required positional argument. File path to hash. |
| \-a, \--algorithm | Algorithm Selection | Selects digest algorithm (sha256, sha512, sha1, md5, argon2). Defaults to sha256. |
| \-s | Salt File | Optional path to file containing salt bytes. |
| \-p | Pepper File | Optional path to file containing pepper bytes. |
| \-h, \--help | Help Menu | Displays usage parameters and exits. |

## **Execution Output Examples**

Standard SHA256 hashing without salt or pepper:

`$ ./hashbin file.txt`  
`SHA256(file.txt,,)=1063bb8cda8b92004f1750a1561b3f1653c7d0565e6c0092b8b873d1a0162f49`

SHA256 hashing with a salt file:

`$ ./hashbin file.txt -s salt.bin`  
`SHA256(file.txt,salt.bin,)=9d9bb9fd15011c252fa146e88b165074dbbd17b511e772d1719ebff8fe9ee497`

SHA256 hashing with both salt and pepper files:

`$ ./hashbin file.txt -s salt.bin -p pepper.bin`  
`SHA256(file.txt,salt.bin,pepper.bin)=f7cf19a8579151ff34cca2a05600221f522b14dd25e75c4948ee962060875aeb`