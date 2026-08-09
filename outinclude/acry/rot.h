#pragma once
#include <acry/base.h>

/* base.h
typedef struct 
{
    HANDLE_t handle;
    ssize_t amount_to_rotate;
} ROTATION_HANDLE;
*/

ROTATION_HANDLE ROTATION_HANDLE_open(ssize_t rotamount);
char* ROTATION_string(ROTATION_HANDLE rot, char* string);
char* ROTATION_string_numerical(ROTATION_HANDLE rot, char* string);
void ROTATION_HANDLE_change(ROTATION_HANDLE *rot, ssize_t rotamount);
int ROTATION_HANDLE_close(ROTATION_HANDLE rot);

char* ROTATION_string_NO_HANDLE(ssize_t rotamount, char* string);
char* ROTATION_string_numerical_NO_HANDLE(ssize_t rotamount, char* string);