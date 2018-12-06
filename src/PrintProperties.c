#include "WinInternal.h"
#include <stdio.h>
#include <sddl.h>
#include "Functions.h"

#define SEC_INFO   ( OWNER_SECURITY_INFORMATION \
                   | GROUP_SECURITY_INFORMATION \
                   | DACL_SECURITY_INFORMATION \
                   | SACL_SECURITY_INFORMATION )

#define STATUS_BUFFER_TOO_SMALL (long)0xC0000023L

ULONG SecurityDescriptorString(
    TRACEHANDLE TraceHandle)
{
    BOOL bRes = 0;
    NTSTATUS Status;
    PWSTR StringSecurityDescriptor = NULL;
    PEVENT_TRACE_SESSION_SECURITY_INFORMATION SessionInfo = NULL;
    ULONG ReturnLen, InfoLen = sizeof(*SessionInfo);

    while (TRUE)
    {
        if (SessionInfo)
            free(SessionInfo);
        SessionInfo = malloc(InfoLen);
        if (!SessionInfo)
            break;
        SessionInfo->EventTraceInformationClass = EventTraceSessionSecurityInformation;
        SessionInfo->TraceHandle = TraceHandle; //Properties->Wnode.HistoricalContext;
        SessionInfo->SecurityInformation = DACL_SECURITY_INFORMATION; //4u
        Status = NtQuerySystemInformation(
            SystemPerformanceTraceInformation,
            SessionInfo,
            InfoLen,
            &ReturnLen);
        InfoLen = ReturnLen;

        if (Status != STATUS_BUFFER_TOO_SMALL)
        {
            if (Status >= S_OK)
            {
                bRes = ConvertSecurityDescriptorToStringSecurityDescriptorW(
                    SessionInfo->SecurityDescriptor,
                    SDDL_REVISION_1,
                    SEC_INFO,
                    &StringSecurityDescriptor,
                    NULL);
                if (bRes)
                    wprintf(L"Session Security:       %ls\n", StringSecurityDescriptor);
            }

            free(SessionInfo);
            return bRes;
        }
    }
    return bRes;
}

void PrintTraceProperties(
    struct _EVENT_TRACE_PROPERTIES_V2* Properties)
{
    PWCHAR ClockType;
    WCHAR Guid[GUID_STRING];

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

    if (Properties->Wnode.ClientContext == EVENT_TRACE_CLOCK_PERFCOUNTER)
        ClockType = L"ClockType:              PerfCounter\n";
    else if (Properties->Wnode.ClientContext == EVENT_TRACE_CLOCK_SYSTEMTIME)
        ClockType = L"ClockType:              SystemTime\n";
    else
    {
        ClockType = L"ClockType               CPU Cycle\n";
        if (Properties->Wnode.ClientContext != EVENT_TRACE_CLOCK_CPUCYCLE)
            ClockType = L"ClockType:              Unknown\n";
    }
    wprintf(ClockType);

    if (Properties->LogFileMode & EVENT_TRACE_NO_PER_PROCESSOR_BUFFERING)
        wprintf(L"No per-processor buffering\n");

    if (Properties->LogFileMode & (EVENT_TRACE_STOP_ON_HYBRID_SHUTDOWN | EVENT_TRACE_PERSIST_ON_HYBRID_SHUTDOWN))
    {
        PWSTR Shutdown = L"Persist";
        if (Properties->LogFileMode & EVENT_TRACE_STOP_ON_HYBRID_SHUTDOWN)
            Shutdown = L"Stop";
        wprintf(L"Hybrid Shutdown:        %ls\n", Shutdown);
    }

    if (Properties->MaximumFileSize)
    {
        PWSTR MaximumFileSize = L"Maximum File Size:      %ld Kb\n";
        if (!(Properties->LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE))
            MaximumFileSize = L"Maximum File Size:      %ld Mb\n";
        wprintf(MaximumFileSize, Properties->MaximumFileSize);
    }
    else
        wprintf(L"Maximum File Size:      not set\n");

    if (Properties->FlushTimer)
    {
        PWSTR unit = L"secs";
        if ((Properties->LogFileMode >> 4) & 1)
            unit = L"ms";
        wprintf(L"Buffer Flush Timer:     %ld %ls\n", Properties->FlushTimer, unit);
    }
    else
        wprintf(L"Buffer Flush Timer:     not set\n");

    if (Properties->LogFileNameOffset > 0)
        wprintf(L"Log Filename:           %ls\n", (PWCHAR)((PBYTE)Properties + Properties->LogFileNameOffset));
    wprintf(L"\n");
}
