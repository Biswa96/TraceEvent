#define _CRT_SECURE_NO_WARNINGS
#include "TraceEvent.h"
#include <stdio.h>

int main(void)
{
    int wargc;
    PWCHAR* wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);

    if (wargc < 3)
    {
        wprintf(
            L"Provide a valid option and agrument.\n"
            L"Try \"TraceEvent.exe help command\" for more information.\n");
        return 0;
    }

    if (!wcscmp(L"start", wargv[1]))
    {
        GUID ProviderID;

        if (guid_from_string(wargv[3], &ProviderID))
            StartSession(wargv[2], ProviderID);
        else
            wprintf(L"Enter Provider GUID correctly\n");

        return 0;
    }
    else if (!wcscmp(L"log", wargv[1]))
    {
        ConsumeEvent(wargv[2]);
    }
    else if (!wcscmp(L"stop", wargv[1]))
    {
        StopSession(wargv[2]);
    }
    else if (!wcscmp(L"help", wargv[1]))
    {
        Usage(wargv[0]);
    }
    else
    {
        wprintf(
            L"Not a valid option or argument.\n"
            L"Try \"TraceEvent.exe help command\" for more information.\n");
    }
}
