#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define GUID_STRING 40

int
WINAPI
guid_from_string(PWSTR s, GUID* id);

void
WINAPI
GuidToString(GUID* id, PWSTR string);

void
WINAPI
Log(ULONG Result, PWSTR Function);

void
WINAPI
Usage(void);

#endif //FUNCTIONS_H
