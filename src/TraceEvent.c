#include "TraceEvent.h"
#include <stdio.h>

#define MAX_LOGGER_SIZE 0x400
static const ULONG BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + MAX_LOGGER_SIZE * sizeof(WCHAR);

ULONG StartSession(PWCHAR LoggerName, GUID ProviderID)
{
    ULONG result;
    TRACEHANDLE hTrace;
    WCHAR Guid[GUID_STRING];

    PEVENT_TRACE_PROPERTIES Properties = (PEVENT_TRACE_PROPERTIES)malloc(BufferSize);
    memset(Properties, 0, BufferSize);
    Properties->Wnode.BufferSize = BufferSize;
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    Properties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    Properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

    result = StartTraceW(&hTrace, LoggerName, Properties);
    if (result)
    {
        wprintf(
            L"Could not start \"%ls\" to logger\n",
            (PWCHAR)((PBYTE)Properties + Properties->LoggerNameOffset));
        GetFormattedMessage(result);
    }
    else
    {
        wprintf(L"Logger Started...\nEnabling \"%ls\" to logger %I64d\n",
            (PWCHAR)((PBYTE)Properties + Properties->LoggerNameOffset), hTrace);
        GetFormattedMessage(result);
    }

    result = EnableTraceEx2(
        hTrace, &ProviderID, EVENT_CONTROL_CODE_ENABLE_PROVIDER,
        TRACE_LEVEL_VERBOSE, 0, 0, 0, NULL);
    if (result)
    {
        wprintf(L"ERROR: Failed to enable Guid: ");
        PrintGuid(&ProviderID, Guid);
        wprintf(L"%ls\n", &Guid);
    }
    else
    {
        wprintf(L"Enabled logger...\n");
        PrintTraceProperties(Properties);
    }

    free(Properties);
    return result;
}

ULONG ConsumeEvent(PWCHAR LoggerName)
{
    ULONG result;
    TRACEHANDLE hTrace;

    EVENT_TRACE_LOGFILEW LogFile = { 0 };
    LogFile.LoggerName = LoggerName;
    LogFile.EventRecordCallback = (PEVENT_RECORD_CALLBACK)EventRecordCallback;
    LogFile.BufferCallback = (PEVENT_TRACE_BUFFER_CALLBACKW)BufferCallback;
    LogFile.LogFileMode = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_NO_PER_PROCESSOR_BUFFERING;

    hTrace = OpenTraceW(&LogFile);
    if (hTrace == (TRACEHANDLE)INVALID_HANDLE_VALUE)
    {
        result = GetLastError();
        wprintf(L"OpenTrace Error: %lu\n", result);
        GetFormattedMessage(result);
        return 0;
    }

    result = ProcessTrace(&hTrace, 1, 0, 0);
    if (result)
    {
        wprintf(L"ProcessTrace Error: %lu\n", result);
        GetFormattedMessage(result);
    }

    return result;
}

ULONG StopSession(PWCHAR LoggerName)
{
    PEVENT_TRACE_PROPERTIES Properties = (PEVENT_TRACE_PROPERTIES)malloc(BufferSize);
    memset(Properties, 0, BufferSize);
    Properties->Wnode.BufferSize = BufferSize;

    ULONG result = ControlTraceW((TRACEHANDLE)NULL, LoggerName, Properties, EVENT_TRACE_CONTROL_STOP);
    if (result == 0)
    {
        wprintf(L"\"%ls\" session stopped succesfully\n",
            (PWCHAR)((PBYTE)Properties + Properties->LoggerNameOffset));
        PrintTraceProperties(Properties);
    }
    else
    {
        wprintf(L"ControlTraceW Error: %ld\n", result);
        GetFormattedMessage(result);
    }

    free(Properties);
    return result;
}
