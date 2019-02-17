#include "WinInternal.h"
#include <tdh.h>
#include "CallBacks.h"
#include "PrintProperties.h"
#include "Log.h"
#include "SecHost.h"
#include <stdio.h>

#define MAX_SESSION_NAME_LEN (1024 * sizeof (wchar_t))
#define MAX_LOGFILE_PATH_LEN (1024 * sizeof (wchar_t))

static const size_t BufferSize = sizeof (EVENT_TRACE_PROPERTIES_V2) + MAX_SESSION_NAME_LEN;

ULONG
WINAPI
StartSession(PWSTR LoggerName, GUID* ProviderID)
{
    HANDLE HeapHandle = GetProcessHeap();
    PEVENT_TRACE_PROPERTIES_V2 Properties = NULL;

    Properties = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, BufferSize);
    Properties->Wnode.BufferSize = (ULONG)BufferSize;
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES;
    Properties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    Properties->LoggerNameOffset = sizeof (*Properties);

    TRACEHANDLE TraceHandle = INFINITE;
    ULONG result = INFINITE;
    result = XYZstartTraceW(&TraceHandle, LoggerName, Properties);
    Log(result, L"StartTraceW Status:     ");

    if (result == ERROR_SUCCESS)
    {
        wprintf(L"Logger Started...\n"
                L"Enabling \"%ls\" to logger %I64d\n",
                (PWCHAR)((PBYTE)Properties + Properties->LoggerNameOffset), TraceHandle);
    }
    else
    {
        wprintf(L"Could not start \"%ls\" to logger\n",
                (PWCHAR)((PBYTE)Properties + Properties->LoggerNameOffset));
    }

    result = EnableTraceEx2(TraceHandle,
                            ProviderID,
                            EVENT_CONTROL_CODE_ENABLE_PROVIDER,
                            TRACE_LEVEL_VERBOSE,
                            0, 0, 0, NULL);

    if (result == ERROR_SUCCESS)
    {
        wprintf(L"Enabled logger...\n");
        PrintTraceProperties(Properties);
    }
    else
    {
        wchar_t Guid[GUID_STRING];
        GuidToString(ProviderID, Guid);
        wprintf(L"ERROR: Failed to enable Guid: %ls\n", &Guid);
    }

    RtlFreeHeap(HeapHandle, 0, Properties);
    return result;
}

ULONG
WINAPI
ConsumeEvent(PWSTR LoggerName)
{
    EVENT_TRACE_LOGFILEW LogFile;
    RtlZeroMemory(&LogFile, sizeof LogFile);

    LogFile.LoggerName = LoggerName;
    LogFile.EventRecordCallback = EventRecordCallback;
    LogFile.BufferCallback = BufferCallback;
    LogFile.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;

    TRACEHANDLE TraceHandle = INFINITE;
    TraceHandle = OpenTraceW(&LogFile);

    if (TraceHandle == INVALID_PROCESSTRACE_HANDLE)
    {
        Log(GetLastError(), L"OpenTraceW Status:      ");
        return 0;
    }

    ULONG result = INFINITE;
    result = ProcessTrace(&TraceHandle, 1, NULL, NULL);
    if (result != ERROR_SUCCESS)
        Log(result, L"ProcessTrace Status:    ");

    return result;
}

ULONG
WINAPI
StopSession(PWSTR LoggerName)
{
    HANDLE HeapHandle = GetProcessHeap();
    PEVENT_TRACE_PROPERTIES_V2 Properties = NULL;
    Properties = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, BufferSize);

    Properties->Wnode.BufferSize = (ULONG)BufferSize;
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES;

    ULONG result = INFINITE;
    result = XYZcontrolTraceW(0, LoggerName, Properties, EVENT_TRACE_CONTROL_STOP);
    Log(result, L"ControlTraceW Status:   ");

    if (result == ERROR_SUCCESS)
    {
        wprintf(L"The \"%ls\" session has been stopped\n", LoggerName);
        PrintTraceProperties(Properties);
    }
    else
        wprintf(L"The \"%ls\" session has not been stopped\n", LoggerName);

    RtlFreeHeap(HeapHandle, 0, Properties);
    return result;
}

ULONG
WINAPI
QuerySession(PWSTR LoggerName)
{
    HANDLE HeapHandle = GetProcessHeap();
    PEVENT_TRACE_PROPERTIES_V2 Properties = NULL;
    Properties = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, BufferSize);

    Properties->Wnode.BufferSize = (ULONG)BufferSize;
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES;

    ULONG result = INFINITE;
    result = XYZcontrolTraceW(0, LoggerName, Properties, EVENT_TRACE_CONTROL_QUERY);

    Log(result, L"ControlTraceW Status:   ");
    if (result == ERROR_SUCCESS)
        PrintTraceProperties(Properties);

    RtlFreeHeap(HeapHandle, 0, Properties);
    return result;
}

ULONG
WINAPI
ListSessions(void)
{
    HANDLE HeapHandle = GetProcessHeap();
    ULONG EtwpMaxLoggers;
    ULONG result = EtwpCacheMaxLogger(&EtwpMaxLoggers);

    PEVENT_TRACE_PROPERTIES_V2 Properties = NULL;
    Properties = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, BufferSize);

    for (TRACEHANDLE i = 0; i < EtwpMaxLoggers; i++)
    {
        RtlZeroMemory(Properties, BufferSize);
        Properties->Wnode.BufferSize = (ULONG)BufferSize;
        Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES;

        result = XYZcontrolTraceW(i, NULL, Properties, EVENT_TRACE_CONTROL_QUERY);

        if (!result || result == ERROR_MORE_DATA)
            PrintTraceProperties(Properties);
    };

    RtlFreeHeap(HeapHandle, 0, Properties);
    return result;
}

void
WINAPI
EnumGuids(void)
{
    HANDLE HeapHandle = GetProcessHeap();
    PTRACE_GUID_PROPERTIES pProviderProperties = NULL, *pProviders = NULL, *pTemp = NULL;
    ULONG GuidCount = 0, PropertyArrayCount = 0, Enabled = 0, Size = 0;
    wchar_t GuidString[GUID_STRING];

    pProviders = RtlAllocateHeap(HeapHandle, 0, sizeof pProviders);

    ULONG result= INFINITE;
    result = XYZenumerateTraceGuids(pProviders, PropertyArrayCount, &GuidCount);

    if (result == ERROR_MORE_DATA && GuidCount)
    {
        PropertyArrayCount = GuidCount;
        Size = sizeof (TRACE_GUID_PROPERTIES) * GuidCount;

        pProviderProperties = RtlAllocateHeap(HeapHandle, 0, Size);
        pTemp = RtlReAllocateHeap(HeapHandle, 0, pProviders, GuidCount * (sizeof pProviders));

        pProviders = pTemp;
        pTemp = NULL;

        for (ULONG i = 0; i < GuidCount; i++)
            pProviders[i] = &pProviderProperties[i];

        result = XYZenumerateTraceGuids(pProviders, PropertyArrayCount, &GuidCount);

        if (result == ERROR_SUCCESS || result == ERROR_MORE_DATA)
        {
            wprintf(L"                 Guid                     Enabled  LoggerId  Level  Flags\n"
                    L"-------------------------------------------------------------------------\n");

            for (ULONG i = 0; i < GuidCount; i++)
            {
                GuidToString(&pProviders[i]->Guid, GuidString);

                wprintf(L"%ls     %5ls",
                        GuidString,
                        pProviders[i]->IsEnable ? L"TRUE" : L"FALSE");

                if (pProviders[i]->IsEnable)
                {
                    ++Enabled;

                    wprintf(L"  %5ld    %5ld    %5ld\n",
                            pProviders[i]->LoggerId,
                            pProviders[i]->EnableLevel,
                            pProviders[i]->EnableFlags);
                }
                else
                    wprintf(L"\n");
            }

            wprintf(L"\nTotal Event Providers: %lu Enabled: %lu\n", GuidCount, Enabled);
        }
        else
            Log(result, L"EnumerateTraceGuids Status:   ");
    }
    else
        Log(result, L"EnumerateTraceGuids Status:   ");

    RtlFreeHeap(HeapHandle, 0, pProviders);
    RtlFreeHeap(HeapHandle, 0, pProviderProperties);
}
