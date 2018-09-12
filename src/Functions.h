#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <Windows.h>

#define GUID_STRING 40
#define MsgSize 0x400

BOOL guid_from_string(PWCHAR s, GUID* id);
void PrintGuid(GUID* id, PWCHAR string);
void GetFormattedMessage(ULONG result);
void Usage(PWCHAR prog);

#endif //FUNCTIONS_H
