#include <windows.h>
#include <tdh.h>
#include "CallBacks.h"
#include "PrintProperties.h"
#include "Functions.h"
#include "SecHost.h"
#include <stdio.h>

#define EtwpMaxLoggers 64
#define MAX_LOGGER_SIZE (1024 * sizeof(wchar_t))
#define MAX_LOGFILE_SIZE (1024 * sizeof(wchar_t))

static const size_t BufferSize = sizeof(EVENT_TRACE_PROPERTIES_V2) + MAX_LOGGER_SIZE;

unsigned long StartSession(
    wchar_t* LoggerName,
    struct _GUID* ProviderID)
{
    PEVENT_TRACE_PROPERTIES_V2 Properties = malloc(BufferSize);
    memset(Properties, 0, BufferSize);
    Properties->Wnode.BufferSize = (ULONG)BufferSize;
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES;
    Properties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    Properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES_V2);

    TRACEHANDLE TraceHandle = 0;
    ULONG result = X_StartTraceW(
        &TraceHandle,
        LoggerName,
        Properties);

    if (result == ERROR_SUCCESS)
    {
        wprintf(L"Logger Started...\nEnabling \"%ls\" to logger %I64d\n",
            (PWCHAR)((PBYTE)Properties + Properties->LoggerNameOffset), TraceHandle);
        Log(result, L"StartTraceW Status:     ");
    }
    else
    {
        wprintf(
            L"Could not start \"%ls\" to logger\n",
            (PWCHAR)((PBYTE)Properties + Properties->LoggerNameOffset));
        Log(result, L"StartTraceW Status:     ");
    }

    result = EnableTraceEx2(
        TraceHandle, ProviderID, EVENT_CONTROL_CODE_ENABLE_PROVIDER,
        TRACE_LEVEL_VERBOSE, 0, 0, 0, NULL);
    if (result == ERROR_SUCCESS)
    {
        wprintf(L"Enabled logger...\n");
        PrintTraceProperties(Properties);
    }
    else
    {
        WCHAR Guid[GUID_STRING];
        wprintf(L"ERROR: Failed to enable Guid: ");
        PrintGuid(ProviderID, Guid);
        wprintf(L"%ls\n", &Guid);
    }

    free(Properties);
    return result;
}

unsigned long ConsumeEvent(
    wchar_t* LoggerName)
{
    EVENT_TRACE_LOGFILEW LogFile = { 0 };
    LogFile.LoggerName = LoggerName;
    LogFile.EventRecordCallback = (PEVENT_RECORD_CALLBACK)EventRecordCallback;
    LogFile.BufferCallback = (PEVENT_TRACE_BUFFER_CALLBACKW)BufferCallback;
    LogFile.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;

    TRACEHANDLE TraceHandle = OpenTraceW(&LogFile);
    if (TraceHandle == (TRACEHANDLE)INVALID_HANDLE_VALUE)
    {
        Log(GetLastError(), L"OpenTraceW Status:      ");
        return 0;
    }

    ULONG result = ProcessTrace(&TraceHandle, 1, NULL, NULL);
    if (result != ERROR_SUCCESS)
        Log(result, L"ProcessTrace Status:    ");

    return result;
}

unsigned long StopSession(
    wchar_t* LoggerName)
{
    PEVENT_TRACE_PROPERTIES_V2 Properties = malloc(BufferSize);
    memset(Properties, 0, BufferSize);
    Properties->Wnode.BufferSize = (ULONG)BufferSize;
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES;

    ULONG result = ControlTraceW(
        (TRACEHANDLE)NULL,
        LoggerName,
        (PEVENT_TRACE_PROPERTIES)Properties,
        EVENT_TRACE_CONTROL_STOP);

    Log(result, L"ControlTraceW Status:   ");
    if (result == ERROR_SUCCESS)
    {
        wprintf(L"\"%ls\" session stopped succesfully\n",
            (PWCHAR)((PBYTE)Properties + Properties->LoggerNameOffset));
        PrintTraceProperties(Properties);
    }

    free(Properties);
    return result;
}

unsigned long QuerySession(
    wchar_t* LoggerName)
{
    PEVENT_TRACE_PROPERTIES_V2 Properties = malloc(BufferSize);
    memset(Properties, 0, BufferSize);
    Properties->Wnode.BufferSize = (ULONG)BufferSize;
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES;

    ULONG result = ControlTraceW(
        (TRACEHANDLE)NULL,
        LoggerName,
        (PEVENT_TRACE_PROPERTIES)Properties,
        EVENT_TRACE_CONTROL_QUERY);

    Log(result, L"ControlTraceW Status:   ");
    if (result == ERROR_SUCCESS)
        PrintTraceProperties(Properties);

    free(Properties);
    return result;
}

void ListSessions(
    void)
{
    ULONG result = 0;
    PEVENT_TRACE_PROPERTIES_V2 Properties = malloc(sizeof(*Properties));

    for (int i = 0; i < EtwpMaxLoggers; i++)
    {
        memset(Properties, 0, sizeof(*Properties));
        Properties->Wnode.BufferSize = (ULONG)sizeof(*Properties);

        result = ControlTraceW(
            (TRACEHANDLE)i,
            NULL,
            (PEVENT_TRACE_PROPERTIES)Properties,
            EVENT_TRACE_CONTROL_QUERY);

        if (!result || result == ERROR_MORE_DATA)
            PrintTraceProperties(Properties);
        i++;
    };

    free(Properties);
}
