#include <Windows.h>
#include <stdio.h>

#define REQUIRED_FORMAT_FLAG ( FORMAT_MESSAGE_ALLOCATE_BUFFER |\
                               FORMAT_MESSAGE_FROM_SYSTEM     |\
                               FORMAT_MESSAGE_IGNORE_INSERTS )

void
WINAPI
Log(ULONG Result, PWSTR Function)
{
    PWSTR MsgBuffer = NULL;
    ULONG Chars = 0;

    Chars = FormatMessageW(REQUIRED_FORMAT_FLAG,
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
            L"  -h,  --help                              Display this usage information.\n"
            L"\n");
}
