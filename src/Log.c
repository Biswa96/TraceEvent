#include <Windows.h>
#include <stdio.h>

#define GUID_STRING 40

// From ReactOS dll/win32/ole32/compobj.c

static inline int is_valid_hex(wchar_t c)
{
    if (!(((c >= '0') && (c <= '9')) ||
        ((c >= 'a') && (c <= 'f')) ||
        ((c >= 'A') && (c <= 'F'))))
        return FALSE;
    return TRUE;
}

static const unsigned char guid_conv_table[256] =
{
    0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x00 */
    0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x10 */
    0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x20 */
    0,   1,   2,   3,   4,   5,   6, 7, 8, 9, 0, 0, 0, 0, 0, 0, /* 0x30 */
    0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x40 */
    0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x50 */
    0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf                             /* 0x60 */
};

/* conversion helper for CLSIDFromString/IIDFromString */
int
WINAPI
guid_from_string(PWSTR s, GUID* id)
{
    int i;

    if (!s || s[0] != '{') {
        memset(id, 0, sizeof(GUID));
        if (!s) return TRUE;
        return FALSE;
    }

    /* in form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} */

    id->Data1 = 0;
    for (i = 1; i < 9; i++) {
        if (!is_valid_hex(s[i])) return FALSE;
        id->Data1 = (id->Data1 << 4) | guid_conv_table[s[i]];
    }
    if (s[9] != '-') return FALSE;

    id->Data2 = 0;
    for (i = 10; i < 14; i++) {
        if (!is_valid_hex(s[i])) return FALSE;
        id->Data2 = (id->Data2 << 4) | guid_conv_table[s[i]];
    }
    if (s[14] != '-') return FALSE;

    id->Data3 = 0;
    for (i = 15; i < 19; i++) {
        if (!is_valid_hex(s[i])) return FALSE;
        id->Data3 = (id->Data3 << 4) | guid_conv_table[s[i]];
    }
    if (s[19] != '-') return FALSE;

    for (i = 20; i < 37; i += 2) {
        if (i == 24) {
            if (s[i] != '-') return FALSE;
            i++;
        }
        if (!is_valid_hex(s[i]) || !is_valid_hex(s[i + 1])) return FALSE;
        id->Data4[(i - 20) / 2] = guid_conv_table[s[i]] << 4 | guid_conv_table[s[i + 1]];
    }

    if (s[37] == '}' && s[38] == '\0')
        return TRUE;

    return FALSE;
}

void
WINAPI
GuidToString(GUID* id, PWSTR string)
{
    _snwprintf_s(string,
                 GUID_STRING,
                 GUID_STRING,
                 L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                 id->Data1, id->Data2, id->Data3,
                 id->Data4[0], id->Data4[1], id->Data4[2],
                 id->Data4[3], id->Data4[4], id->Data4[5],
                 id->Data4[6], id->Data4[7]);
}

void
WINAPI
Log(ULONG Result, PWSTR Function)
{
    PWSTR MsgBuffer = NULL;
    ULONG Chars = 0;
    ULONG Flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

    Chars = FormatMessageW(Flags,
                           NULL,
                           Result,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (PWSTR)&MsgBuffer,
                           0,
                           NULL);

    if (Chars)
        wprintf(L"%ls%ld\t %ls", Function, (Result & 0xFFFF), MsgBuffer);
    else
        wprintf(L"%ls%ld\n", Function, (Result & 0xFFFF));

    LocalFree(MsgBuffer);
}

void
WINAPI
Usage(void)
{
    wprintf(L"\nTraceEvent -- (c) Copyright 2018-19 Biswapriyo Nath\n"
            L"Licensed under GNU Public License version 3 or higher\n\n"
            L"Trace and log events in real time sessions\n"
            L"Usage: TraceEvent.exe [--] [option] [argument]\n\n"
            L"Options:\n"
            L"  -E,  --enumguidinfo                      Enumerate registered trace GUIDs with all PID and Logger ID.\n"
            L"  -e,  --enumguid                          Enumerate registered trace GUIDs.\n"
            L"  -g,  --guid        <ProviderGUID>        Add Event Provider GUID with trace session.\n"
            L"  -L,  --list                              List all trace sessions.\n"
            L"  -l,  --log         <LoggerName>          Log events in real time.\n"
            L"  -q,  --query       <LoggerName>          Query status of <LoggerName> trace session.\n"
            L"  -S,  --start       <LoggerName>          Starts the <LoggerName> trace session.\n"
            L"  -s,  --stop        <LoggerName>          Stops the <LoggerName> trace session.\n"
            L"  -h,  --help                              Display usage information.\n"
            L"\n");
}
