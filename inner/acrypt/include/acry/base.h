#pragma once

#if defined(_WIN32) || defined(_WIN64)
typedef void* HANDLE_t;
#else
typedef int* HANDLE_t;
#endif

typedef struct 
{
    HANDLE_t handle;
    void* bytes;
} RANDOM_HANDLE;