#include "PrintProperties.h"
#include <stdio.h>
#include <sddl.h>

#define SEC_INFO   ( OWNER_SECURITY_INFORMATION \
                   | GROUP_SECURITY_INFORMATION  \
                   | DACL_SECURITY_INFORMATION    \
                   | SACL_SECURITY_INFORMATION )

#define STATUS_BUFFER_TOO_SMALL 0xC0000023L

typedef enum _EVENT_TRACE_INFORMATION_CLASS {
    EventTraceSessionSecurityInformation = 4
} EVENT_TRACE_INFORMATION_CLASS;

typedef struct _EVENT_TRACE_SESSION_SECURITY_INFORMATION {
    EVENT_TRACE_INFORMATION_CLASS EventTraceInformationClass;
    ULONG SecurityInformation;
    ULONG64 TraceHandle;
    SECURITY_DESCRIPTOR SecurityDescriptor[ANYSIZE_ARRAY];
} EVENT_TRACE_SESSION_SECURITY_INFORMATION, *PEVENT_TRACE_SESSION_SECURITY_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemPerformanceTraceInformation = 31
} SYSTEM_INFORMATION_CLASS;

NTSTATUS NtQuerySystemInformation(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

ULONG SecurityDescriptorString(ULONG64 TraceHandle)
{
    ULONG result = 0, ReturnLen = 0;
    ULONG InfoLen = sizeof(EVENT_TRACE_SESSION_SECURITY_INFORMATION);
    NTSTATUS status;
    PWCHAR wstring = NULL;
    PEVENT_TRACE_SESSION_SECURITY_INFORMATION SessionInfo = NULL;

    while (TRUE)
    {
        if (SessionInfo)
            free(SessionInfo);
        SessionInfo = (PEVENT_TRACE_SESSION_SECURITY_INFORMATION)malloc(InfoLen);
        if (!SessionInfo)
            break;
        SessionInfo->EventTraceInformationClass = EventTraceSessionSecurityInformation;
        SessionInfo->TraceHandle = TraceHandle; //Properties->Wnode.HistoricalContext;
        SessionInfo->SecurityInformation = DACL_SECURITY_INFORMATION; //0x4
        status = NtQuerySystemInformation(
            SystemPerformanceTraceInformation,
            SessionInfo, InfoLen, &ReturnLen);
        InfoLen = ReturnLen;

        if (status != STATUS_BUFFER_TOO_SMALL)
        {
            if (status >= S_OK)
            {
                result = ConvertSecurityDescriptorToStringSecurityDescriptorW(
                    SessionInfo->SecurityDescriptor,
                    SDDL_REVISION_1,
                    SEC_INFO,
                    &wstring,
                    NULL);
                if (result == 1)
                    wprintf(L"Session Security:       %ls\n", wstring);
            }
            free(SessionInfo);
            return result;
        }
    }
    return result;
}

void PrintTraceProperties(PEVENT_TRACE_PROPERTIES Properties)
{
    PWCHAR ClockType, MaximumFileSize, unit;
    WCHAR Guid[GUID_STRING];
    ULONG FlushTimer;

    if(Properties->LoggerNameOffset)
        wprintf(L"Logger Name:            %ls\n", (PWCHAR)((PBYTE)Properties + Properties->LoggerNameOffset));
    wprintf(L"Logger Id:              0x%I64x\n", Properties->Wnode.HistoricalContext);
    wprintf(L"Logger Thread Id:       %p\n", Properties->LoggerThreadId);

    PrintGuid(&Properties->Wnode.Guid, Guid);
    wprintf(L"Guid:                   %ls\n", &Guid);
    SecurityDescriptorString(Properties->Wnode.HistoricalContext);

    if (Properties->BufferSize)
        wprintf(L"Buffer Size:            %ld Kb\n", Properties->BufferSize);
    else
        wprintf(L"Buffer Size:            default value\n");
    if (Properties->MaximumBuffers)
        wprintf(L"Maximum Buffers:        %ld\n", Properties->MaximumBuffers);
    else
        wprintf(L"Maximum Buffers:        default value\n");
    if (Properties->MinimumBuffers)
        wprintf(L"Minimum Buffers:        %ld\n", Properties->MinimumBuffers);
    else
        wprintf(L"Minimum Buffers:        default value\n");
    wprintf(L"Number of Buffers:      %ld\n", Properties->NumberOfBuffers);
    wprintf(L"Free Buffers:           %ld\n", Properties->FreeBuffers);
    wprintf(L"Buffers Written:        %ld\n", Properties->BuffersWritten);
    wprintf(L"Events Lost:            %ld\n", Properties->EventsLost);
    wprintf(L"Log Buffers Lost:       %ld\n", Properties->LogBuffersLost);
    wprintf(L"Real Time Buffers Lost: %ld\n", Properties->RealTimeBuffersLost);
    wprintf(L"Real Time Consumers:    %ld\n", Properties->Wnode.ProviderId);

    if (Properties->Wnode.ClientContext == 1)
        ClockType = L"ClockType:              PerfCounter\n";
    else if (Properties->Wnode.ClientContext == 2)
        ClockType = L"ClockType:              SystemTime\n";
    else
    {
        ClockType = L"ClockType               CPU Cycle\n";
        if (Properties->Wnode.ClientContext != 3)
            ClockType = L"ClockType:              Unknown\n";
    }
    wprintf(ClockType);

    if (Properties->MaximumFileSize)
    {
        MaximumFileSize = L"Maximum File Size:      %ld Kb\n";
        if (!(Properties->LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE))
            MaximumFileSize = L"Maximum File Size:      %ld Mb\n";
        wprintf(MaximumFileSize, Properties->MaximumFileSize);
    }
    else
        wprintf(L"Maximum File Size:      not set\n");

    FlushTimer = Properties->FlushTimer;
    if (FlushTimer)
    {
        unit = L"secs";
        if ((Properties->LogFileMode >> 4) & 1)
            unit = L"ms";
        wprintf(L"Buffer Flush Timer:     %ld %ls\n", FlushTimer, unit);
    }
    else
        wprintf(L"Buffer Flush Timer:     not set\n");

    if (Properties->LogFileNameOffset > 0)
    {
        wprintf(L"Log Filename:           %ls\n",
            (PWCHAR)((PBYTE)Properties + Properties->LogFileNameOffset));
    }
}
