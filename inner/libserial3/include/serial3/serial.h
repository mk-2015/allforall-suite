#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void serial3_handle;

serial3_handle* s3Open(const char* fileDestination, int* error);
int s3Close(serial3_handle* handle);

int s3WriteToFile(serial3_handle* handle, const char* data);
int s3RemoveLine(serial3_handle* handle, int lineNumber, const char* lineEnding);
int s3WriteLine(serial3_handle* handle, int lineNumber, const char* data);
int s3ReadLine(serial3_handle* handle, int lineNumber);
int s3ReadFromFile(serial3_handle* handle);
int s3Flush(serial3_handle* handle);

int s3SerializeString(serial3_handle* handle, const char* key, const char* value);
int s3SerializeInt(serial3_handle* handle, const char* key, int value);
int s3SerializeBool(serial3_handle* handle, const char* key, int value);
int s3DeserializeString(serial3_handle* handle, const char* key, char* out, size_t outSize);
int s3DeserializeInt(serial3_handle* handle, const char* key, int* outValue);
int s3DeserializeBool(serial3_handle* handle, const char* key, int* outValue);

size_t s3GetBufferSize(serial3_handle* handle);
int s3GetBuffer(serial3_handle* handle, char* out, size_t outSize);

#ifdef __cplusplus
}
#endif

#endif // __SERIAL_H__