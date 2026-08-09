#include <acry/random.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <wincrypt.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

RANDOM_HANDLE RANDOM_OPEN_handle(void) {
    RANDOM_HANDLE r;
    r.bytes = NULL;

#if defined(_WIN32) || defined(_WIN64)
    HCRYPTPROV hProv = 0;
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        r.handle = (HANDLE_t)NULL;
        return r;
    }
    r.handle = (HANDLE_t)hProv;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        r.handle = (HANDLE_t)-1;
        return r;
    }
    // Store the file descriptor safely via pointer cast
    int* ptr = malloc(sizeof(int));
    if (ptr) {
        *ptr = fd;
        r.handle = (HANDLE_t)ptr;
    } else {
        close(fd);
        r.handle = (HANDLE_t)-1;
    }
#endif

    return r;
}

void RANDOM_LOAD_bytes(size_t bytes_count, RANDOM_HANDLE *random) {
    if (!random) return;

    // Free any previously allocated bytes block
    if (random->bytes) {
        free(random->bytes);
        random->bytes = NULL;
    }

    if (bytes_count == 0) return;

    void *buf = malloc(bytes_count);
    if (!buf) return;

#if defined(_WIN32) || defined(_WIN64)
    HCRYPTPROV hProv = (HCRYPTPROV)random->handle;
    if (hProv && hProv != (HCRYPTPROV)INVALID_HANDLE_VALUE) {
        if (!CryptGenRandom(hProv, (DWORD)bytes_count, (BYTE*)buf)) {
            free(buf);
            return;
        }
    } else {
        free(buf);
        return;
    }
#else
    if (!random->handle || random->handle == (HANDLE_t)-1) {
        free(buf);
        return;
    }
    int fd = *(int*)random->handle;
    size_t total_read = 0;
    while (total_read < bytes_count) {
        ssize_t n = read(fd, (char*)buf + total_read, bytes_count - total_read);
        if (n <= 0) break;
        total_read += n;
    }
    if (total_read != bytes_count) {
        free(buf);
        return;
    }
#endif

    random->bytes = buf;
}

void* RANDOM_GET_bytes(RANDOM_HANDLE random) {
    return random.bytes;
}

int RANDOM_CLOSE_handle(RANDOM_HANDLE random) {
    if (random.bytes) {
        free(random.bytes);
    }

#if defined(_WIN32) || defined(_WIN64)
    HCRYPTPROV hProv = (HCRYPTPROV)random.handle;
    if (hProv && hProv != (HCRYPTPROV)INVALID_HANDLE_VALUE) {
        CryptReleaseContext(hProv, 0);
        return 0;
    }
#else
    if (random.handle && random.handle != (HANDLE_t)-1) {
        int *ptr = (int*)random.handle;
        close(*ptr);
        free(ptr);
        return 0;
    }
#endif

    return -1;
}

void* RANDOM_GET_bytes_NO_HANDLE(size_t size) {
    if (size == 0) return NULL;

    RANDOM_HANDLE r = RANDOM_OPEN_handle();
    if (!r.handle || r.handle == (HANDLE_t)-1) {
        return NULL;
    }

    RANDOM_LOAD_bytes(size, &r);
    void* data = RANDOM_GET_BYTES(r);

    r.bytes = NULL; 

    RANDOM_CLOSE_handle(r);
    return data;
}