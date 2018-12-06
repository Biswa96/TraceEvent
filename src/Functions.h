#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <wchar.h>

#define GUID_STRING 128u

int guid_from_string(
    wchar_t* s,
    struct _GUID* id);

void PrintGuid(
    struct _GUID* id,
    wchar_t* string);

void Log(
    unsigned long Result,
    wchar_t* Function);

void Usage(
    void);

#endif //FUNCTIONS_H
