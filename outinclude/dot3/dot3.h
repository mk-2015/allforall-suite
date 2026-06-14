#ifndef DOT3_C_H
#define DOT3_C_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void* NewRegexHandler(const char* regexString);
bool ParseRegexString(void* handle, const char* StringToParse);
void CloseRegexHandler(void* handle);

#ifdef __cplusplus
}
#endif

#endif // DOT3_C_H
