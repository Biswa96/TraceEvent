#include "WinInternal.h"
#include <tdh.h>
#include "CallBacks.h"
#include "PrintProperties.h"
#include "Log.h"
#include "SecHost.h"
#include <stdio.h>

#define MAX_SESSION_NAME_LEN (1024 * sizeof (wchar_t))
#define MAX_LOGFILE_PATH_LEN (1024 * sizeof (wchar_t))

// Global variables
static ULONG result = INFINITE;
static PEVENT_TRACE_PROPERTIES_V2 Properties = NULL;
static const ULONG BufferSize = sizeof (*Properties) + MAX_SESSION_NAME_LEN;

ULONG
WINAPI
StartSession(PWSTR LoggerName, GUID* ProviderID)
{
    HANDLE HeapHandle = GetProcessHeap();
    Properties = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, BufferSize);

    Properties->Wnode.BufferSize = BufferSize;
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES;
    Properties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    Properties->LoggerNameOffset = sizeof (*Properties);

    TRACEHANDLE TraceHandle = INFINITE;
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
        UNICODE_STRING GuidString;
        RtlStringFromGUID(ProviderID, &GuidString);
        wprintf(L"ERROR: Failed to enable Guid: %ls\n", GuidString.Buffer);
        RtlFreeUnicodeString(&GuidString);
    }

    // Cleanup
    if (Properties)
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
    Properties = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, BufferSize);

    Properties->Wnode.BufferSize = BufferSize;
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES;

    result = XYZcontrolTraceW(0, LoggerName, Properties, EVENT_TRACE_CONTROL_STOP);
    Log(result, L"ControlTraceW Status:   ");

    if (result == ERROR_SUCCESS)
    {
        wprintf(L"The \"%ls\" session has been stopped\n", LoggerName);
        PrintTraceProperties(Properties);
    }
    else
        wprintf(L"The \"%ls\" session has not been stopped\n", LoggerName);

    // Cleanup
    if (Properties)
        RtlFreeHeap(HeapHandle, 0, Properties);
    return result;
}

ULONG
WINAPI
QuerySession(PWSTR LoggerName)
{
    HANDLE HeapHandle = GetProcessHeap();
    Properties = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, BufferSize);

    Properties->Wnode.BufferSize = BufferSize;
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES;

    result = XYZcontrolTraceW(0, LoggerName, Properties, EVENT_TRACE_CONTROL_QUERY);

    Log(result, L"ControlTraceW Status:   ");
    if (result == ERROR_SUCCESS)
        PrintTraceProperties(Properties);

    // Cleanup
    if (Properties)
        RtlFreeHeap(HeapHandle, 0, Properties);
    return result;
}

ULONG
WINAPI
ListSessions(void)
{
    HANDLE HeapHandle = GetProcessHeap();
    ULONG EtwpMaxLoggers;
    result = EtwpCacheMaxLogger(&EtwpMaxLoggers);

    Properties = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, BufferSize);

    for (TRACEHANDLE i = 0; i < EtwpMaxLoggers; i++)
    {
        RtlZeroMemory(Properties, BufferSize);
        Properties->Wnode.BufferSize = BufferSize;
        Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES;

        result = XYZcontrolTraceW(i, NULL, Properties, EVENT_TRACE_CONTROL_QUERY);

        if (!result || result == ERROR_MORE_DATA)
            PrintTraceProperties(Properties);
    };

    // Cleanup
    if(Properties)
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
    UNICODE_STRING GuidString;

    pProviders = RtlAllocateHeap(HeapHandle, 0, sizeof pProviders);

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
                RtlStringFromGUID(&pProviders[i]->Guid, &GuidString);

                wprintf(L"%ls     %5ls",
                        GuidString.Buffer,
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

    // Cleanup
    RtlFreeUnicodeString(&GuidString);
    if(pProviders)
        RtlFreeHeap(HeapHandle, 0, pProviders);
    if(pProviderProperties)
        RtlFreeHeap(HeapHandle, 0, pProviderProperties);
}

void
WINAPI
EnumGuidsInfo(void)
{
    ULONG GuidCount = 0, GuidListSize = 0;
    ULONG InfoListSize = 0, RequiredListSize = 0;
    UNICODE_STRING GuidString;

    HANDLE HeapHandle = GetProcessHeap();
    GUID* pGuids = NULL;
    PTRACE_GUID_INFO pInfo = NULL;
    PTRACE_PROVIDER_INSTANCE_INFO pInstance = NULL;
    PTRACE_ENABLE_INFO pEnable = NULL;

    pGuids = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, sizeof (char));

    while (TRUE)
    {
        result = XYZenumerateTraceGuidsEx(TraceGuidQueryList,
                                          NULL,
                                          0,
                                          pGuids,
                                          GuidListSize,
                                          &RequiredListSize);
        GuidListSize = RequiredListSize;

        if (result == ERROR_SUCCESS)
            break;

        GUID* pTemp = RtlReAllocateHeap(HeapHandle,
                                        HEAP_ZERO_MEMORY,
                                        pGuids,
                                        RequiredListSize);
        pGuids = pTemp;
        pTemp = NULL;
    }

    if (result == ERROR_SUCCESS)
    {
        pInfo = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, sizeof (char));
        GuidCount = GuidListSize / sizeof (*pGuids);

        for (ULONG i = 0; i < GuidCount; i++)
        {
            RtlStringFromGUID(&pGuids[i], &GuidString);
            wprintf(L"\n%ls\n", GuidString.Buffer);

            while (TRUE)
            {
                result = XYZenumerateTraceGuidsEx(TraceGuidQueryInfo,
                                                  &pGuids[i],
                                                  sizeof pGuids[i],
                                                  pInfo,
                                                  InfoListSize,
                                                  &RequiredListSize);
                InfoListSize = RequiredListSize;

                if (result == ERROR_SUCCESS)
                    break;

                PTRACE_GUID_INFO pTemp = RtlReAllocateHeap(HeapHandle,
                                                           HEAP_ZERO_MEMORY,
                                                           pInfo,
                                                           RequiredListSize);
                pInfo = pTemp;
                pTemp = NULL;
            }

            if (result == ERROR_SUCCESS)
            {
                pInstance = (PTRACE_PROVIDER_INSTANCE_INFO)((PBYTE)pInfo + sizeof (*pInfo));

                for (ULONG j = 0; j < pInfo->InstanceCount; j++)
                {
                    wprintf(L"\tPID %lu\n", pInstance->Pid);

                    if (pInstance->EnableCount > 0)
                    {
                        pEnable = (PTRACE_ENABLE_INFO)((PBYTE)pInstance + sizeof (*pInstance));

                        for (ULONG k = 0; k < pInstance->EnableCount; k++)
                        {
                            wprintf(L"\t\tLoggerId: %hu\n", pEnable->LoggerId);
                            ++pEnable;
                        }
                    }

                    pInstance = (PTRACE_PROVIDER_INSTANCE_INFO)((PBYTE)pInstance + pInstance->NextOffset);
                }
            }
            else
                Log(result, L"EnumerateTraceGuidsEx(TraceGuidQueryInfo) Status: ");
        }

        wprintf(L"\nTotal Event Providers: %lu\n", GuidCount);
    }
    else
        Log(result, L"EnumerateTraceGuidsEx(TraceGuidQueryList) Status: ");

    // Cleanup
    RtlFreeUnicodeString(&GuidString);
    if (pGuids)
        RtlFreeHeap(HeapHandle, 0, pGuids);
    if (pInfo)
        RtlFreeHeap(HeapHandle, 0, pInfo);
}
