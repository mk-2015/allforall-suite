#include <acry/rot.h>
#include <stdlib.h>
#include <string.h>

ROTATION_HANDLE ROTATION_HANDLE_open(ssize_t rotamount) {
    ROTATION_HANDLE rot;
    rot.amount_to_rotate = rotamount;
    rot.handle = (HANDLE_t)NULL;
    return rot;
}

char* ROTATION_string(ROTATION_HANDLE rot, char* string) {
    if (!string) return NULL;

    size_t len = strlen(string);
    char *result = malloc(len + 1);
    if (!result) return NULL;

    strcpy(result, string);
    ssize_t shift = rot.amount_to_rotate % 26;
    if (shift < 0) shift += 26;

    for (size_t i = 0; i < len; i++) {
        char c = result[i];
        if (c >= 'a' && c <= 'z') {
            result[i] = 'a' + (c - 'a' + shift) % 26;
        } else if (c >= 'A' && c <= 'Z') {
            result[i] = 'A' + (c - 'A' + shift) % 26;
        }
    }

    return result;
}

char* ROTATION_string_numerical(ROTATION_HANDLE rot, char* string) {
    if (!string) return NULL;

    size_t len = strlen(string);
    char *result = malloc(len + 1);
    if (!result) return NULL;

    strcpy(result, string);
    ssize_t shift = rot.amount_to_rotate % 10;
    if (shift < 0) shift += 10;

    for (size_t i = 0; i < len; i++) {
        char c = result[i];
        if (c >= '0' && c <= '9') {
            result[i] = '0' + (c - '0' + shift) % 10;
        }
    }

    return result;
}

void ROTATION_HANDLE_change(ROTATION_HANDLE *rot, ssize_t rotamount) {
    if (rot) {
        rot->amount_to_rotate = rotamount;
    }
}

int ROTATION_HANDLE_close(ROTATION_HANDLE rot) {
    (void)rot;
    return 0;
}

char* ROTATION_string_NO_HANDLE(ssize_t rotamount, char* string) {
    ROTATION_HANDLE rot = ROTATION_HANDLE_open(rotamount);
    char *res = ROTATION_string(rot, string);
    ROTATION_HANDLE_close(rot);
    return res;
}

char* ROTATION_string_numerical_NO_HANDLE(ssize_t rotamount, char* string) {
    ROTATION_HANDLE rot = ROTATION_HANDLE_open(rotamount);
    
    if (!string) {
        ROTATION_HANDLE_close(rot);
        return NULL;
    }

    size_t len = strlen(string);
    char *result = malloc(len + 1);
    if (!result) {
        ROTATION_HANDLE_close(rot);
        return NULL;
    }

    strcpy(result, string);
    ssize_t shift = rot.amount_to_rotate % 10;
    if (shift < 0) shift += 10;

    for (size_t i = 0; i < len; i++) {
        char c = result[i];
        if (c >= '0' && c <= '9') {
            result[i] = '0' + (c - '0' + shift) % 10;
        }
    }

    ROTATION_HANDLE_close(rot);
    return result;
}