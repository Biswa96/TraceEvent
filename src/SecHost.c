#include "WinInternal.h"
#include "Helpers.h"
#include <strsafe.h>

static const GUID CKCLGuid = {
    0x54DEA73A,
    0xED1F,
    0x42A4,
    { 0xAF, 0x71, 0x3E, 0x63, 0xD0, 0x56, 0xF1, 0x74 } };

static const GUID SystemTraceControl = {
    0x9E814AAD,
    0x3204,
    0x11D2,
    { 0x9A, 0x82, 0x00, 0x60, 0x08, 0xA8, 0x69, 0x39 } };

ULONG
WINAPI
EtwpCacheMaxLogger(PULONG EtwpMaxLoggers)
{
    NTSTATUS Status;
    ULONG ReturnLength;

    Status = NtTraceControl(TraceControlMaxLoggers,
                            NULL,
                            0,
                            EtwpMaxLoggers,
                            sizeof (*EtwpMaxLoggers),
                            &ReturnLength);

    return RtlNtStatusToDosError(Status);
}

ULONG
WINAPI
XYZstartTraceW(PTRACEHANDLE TraceHandle,
               PWSTR InstanceName,
               PEVENT_TRACE_PROPERTIES_V2 Properties)
{
    ULONG FilterDescCount, ReturnedLength, LastError;
    ULONG InstanceNameLength = INFINITE, LogFileNameLength = INFINITE, BufferSize = 0;
    BOOLEAN IsLogFilePresent;
    PEVENT_FILTER_DESCRIPTOR FilterDesc = NULL;
    PWSTR LogFileName = NULL;

    if (!Properties || !TraceHandle)
    {
        RtlSetLastWin32Error(ERROR_INVALID_PARAMETER);
        return ERROR_INVALID_PARAMETER;
    }
    if (!InstanceName)
    {
        RtlSetLastWin32Error(ERROR_INVALID_NAME);
        return ERROR_INVALID_NAME;
    }

    LastError = EtwpValidateTraceProperties(Properties,
                                            &FilterDescCount,
                                            &FilterDesc,
                                            &ReturnedLength);
    if (LastError)
    {
        RtlSetLastWin32Error(LastError);
        return LastError;
    }

    if (_wcsicmp(InstanceName, L"NT Kernel Logger"))
        Properties->Wnode.Guid = SystemTraceControl;
    if (_wcsicmp(InstanceName, L"Circular Kernel Context Logger"))
        Properties->Wnode.Guid = CKCLGuid;

    do
    {
        ++InstanceNameLength;
    } while (InstanceName[InstanceNameLength]);
    BufferSize = ReturnedLength + (sizeof (wchar_t) * (InstanceNameLength + 1));

    if (Properties->LogFileNameOffset <= 0)
    {
        IsLogFilePresent = FALSE;
    }
    else
    {
        IsLogFilePresent = TRUE;
        LogFileName = (PWSTR)((PBYTE)Properties + Properties->LogFileNameOffset);

        do
        {
            ++LogFileNameLength;
        } while (LogFileName[LogFileNameLength]);

        BufferSize += (sizeof (wchar_t) * (LogFileNameLength + 1));
    }

    if (Properties->Wnode.BufferSize < BufferSize)
    {
        RtlSetLastWin32Error(ERROR_BAD_LENGTH);
        return ERROR_BAD_LENGTH;
    }

    if (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_NEWFILE)
    {
        PWSTR pwc = wcschr(LogFileName, '%');
        if (!pwc || pwc != wcsrchr(LogFileName, '%') || !wcsstr(LogFileName, L"%d"))
        {
            RtlSetLastWin32Error(ERROR_INVALID_NAME);
            return ERROR_INVALID_NAME;
        }
    }

    // Start allocating buffer for NtTraceControl
    HANDLE HeapHandle = GetProcessHeap();
    PWMI_LOGGER_INFORMATION WmiLogInfo = NULL;
    WmiLogInfo = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, Properties->Wnode.BufferSize);

    EtwpCopyPropertiesToInfo(Properties, WmiLogInfo);
    WmiLogInfo->Wnode.Flags |= WNODE_FLAG_TRACED_GUID;
    RtlInitUnicodeString(&WmiLogInfo->LoggerName, InstanceName);
    if (IsLogFilePresent)
        RtlInitUnicodeString(&WmiLogInfo->LogFileName, LogFileName);


    NTSTATUS Status;
    Status = NtTraceControl(TraceControlStartLogger,
                            WmiLogInfo,
                            WmiLogInfo->Wnode.BufferSize,
                            WmiLogInfo,
                            WmiLogInfo->Wnode.BufferSize,
                            &ReturnedLength);

    LastError = RtlNtStatusToDosError(Status);
    if (!LastError)
    {
        ULONG Diff = 0;
        Properties->LoggerNameOffset = ReturnedLength;
        ULONG LoggerNameOffset = Properties->LoggerNameOffset;
        ULONG LogFileNameOffset = Properties->LogFileNameOffset;

        *TraceHandle = WmiLogInfo->Wnode.HistoricalContext;
        EtwpCopyInfoToProperties(WmiLogInfo, Properties);

        if (LoggerNameOffset > LogFileNameOffset)
            LogFileNameOffset = Properties->Wnode.BufferSize;
        Diff = LogFileNameOffset - LoggerNameOffset;

        if (sizeof (wchar_t) * (InstanceNameLength + 1) <= Diff)
            StringCbCopyW((PWSTR)((PBYTE)Properties + LoggerNameOffset), Diff, InstanceName);

        if (LogFileNameOffset > LoggerNameOffset)
            LoggerNameOffset = Properties->Wnode.BufferSize;
        Diff = LoggerNameOffset - LogFileNameOffset;

        if (WmiLogInfo->LogFileName.Length && WmiLogInfo->LogFileName.Length <= Diff)
            StringCbCopyW((PWSTR)((PBYTE)Properties + LogFileNameOffset), Diff, LogFileName);
    }

    // Cleanup
    if(WmiLogInfo)
        RtlFreeHeap(HeapHandle, 0, WmiLogInfo);
    return LastError;
}

ULONG
WINAPI
XYZcontrolTraceW(TRACEHANDLE TraceHandle,
                 PWSTR InstanceName,
                 PEVENT_TRACE_PROPERTIES_V2 Properties,
                 ULONG ControlCode)
{
    ULONG FilterDescCount, ReturnedLength, LastError;
    ULONG InstanceNameLength = INFINITE, LogFileNameLength = INFINITE, BufferSize = 0;
    BOOLEAN IsLogFilePresent;
    PEVENT_FILTER_DESCRIPTOR FilterDesc = NULL;
    PWSTR LogFileName = NULL;

    if (!Properties)
    {
        RtlSetLastWin32Error(ERROR_INVALID_PARAMETER);
        return ERROR_INVALID_PARAMETER;
    }

    LastError = EtwpValidateTraceProperties(Properties,
                                            &FilterDescCount,
                                            &FilterDesc,
                                            &ReturnedLength);
    if (LastError)
    {
        RtlSetLastWin32Error(LastError);
        return LastError;
    }

    if (Properties->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND &&
        ControlCode == EVENT_TRACE_CONTROL_UPDATE &&
        Properties->LogFileNameOffset > 0)
    {
        RtlSetLastWin32Error(ERROR_INVALID_PARAMETER);
        return ERROR_INVALID_PARAMETER;
    }

    if (InstanceName)
    {
        if (_wcsicmp(InstanceName, L"NT Kernel Logger"))
            Properties->Wnode.Guid = SystemTraceControl;
        if (_wcsicmp(InstanceName, L"Circular Kernel Context Logger"))
            Properties->Wnode.Guid = CKCLGuid;

        do
        {
            ++InstanceNameLength;
        } while (InstanceName[InstanceNameLength]);
        BufferSize = ReturnedLength + (sizeof (wchar_t) * (InstanceNameLength + 1));
    }
    else
        InstanceNameLength = 0;

    if (Properties->LogFileNameOffset <= 0)
    {
        IsLogFilePresent = FALSE;
    }
    else
    {
        IsLogFilePresent = TRUE;
        LogFileName = (PWSTR)((PBYTE)Properties + Properties->LogFileNameOffset);
        do
        {
            ++LogFileNameLength;
        } while (LogFileName[LogFileNameLength]);

        BufferSize += (sizeof (wchar_t) * (LogFileNameLength + 1));
    }

    // Start allocating buffer for NtTraceControl
    HANDLE HeapHandle = GetProcessHeap();
    PWMI_LOGGER_INFORMATION WmiLogInfo = NULL;
    WmiLogInfo = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, Properties->Wnode.BufferSize);

    EtwpCopyPropertiesToInfo(Properties, WmiLogInfo);
    WmiLogInfo->Wnode.Flags |= WNODE_FLAG_TRACED_GUID;
    WmiLogInfo->Wnode.HistoricalContext = TraceHandle;
    if (InstanceName)
        RtlInitUnicodeString(&WmiLogInfo->LoggerName, InstanceName);
    if (IsLogFilePresent)
        RtlInitUnicodeString(&WmiLogInfo->LogFileName, LogFileName);

    // Convert ControlCode to Function code for NtTraceControl
    ULONG FunctionCode;
    switch (ControlCode)
    {
    case EVENT_TRACE_CONTROL_QUERY:
        FunctionCode = TraceControlQueryLogger;
        break;

    case EVENT_TRACE_CONTROL_STOP:
        FunctionCode = TraceControlStopLogger;
        break;

    case EVENT_TRACE_CONTROL_UPDATE:
        FunctionCode = TraceControlUpdateLogger;
        break;

    case EVENT_TRACE_CONTROL_FLUSH:
        FunctionCode = TraceControlFlushLogger;
        break;

    case EVENT_TRACE_CONTROL_INCREMENT_FILE:
        FunctionCode = TraceControlIncrementLoggerFile;
        break;

    default:
        FunctionCode = INFINITE;
    }

    NTSTATUS Status;
    Status = NtTraceControl(FunctionCode,
                            WmiLogInfo,
                            WmiLogInfo->Wnode.BufferSize,
                            WmiLogInfo,
                            WmiLogInfo->Wnode.BufferSize,
                            &ReturnedLength);

    LastError = RtlNtStatusToDosError(Status);
    if (!LastError)
    {
        ULONG Diff = 0;
        // To-Be-Fixed:
        // Properties->LoggerNameOffset = ReturnedLength;
        ULONG LoggerNameOffset = Properties->LoggerNameOffset;
        ULONG LogFileNameOffset = Properties->LogFileNameOffset;

        EtwpCopyInfoToProperties(WmiLogInfo, Properties);

        if (LoggerNameOffset > LogFileNameOffset)
            LogFileNameOffset = Properties->Wnode.BufferSize;
        Diff = LogFileNameOffset - LoggerNameOffset;

        if (InstanceName && (sizeof (wchar_t) * (InstanceNameLength + 1) <= Diff) )
            StringCbCopyW((PWSTR)((PBYTE)Properties + LoggerNameOffset), Diff, InstanceName);

        if (LogFileNameOffset > LoggerNameOffset)
            LoggerNameOffset = Properties->Wnode.BufferSize;
        Diff = LoggerNameOffset - LogFileNameOffset;

        if (WmiLogInfo->LogFileName.Length && WmiLogInfo->LogFileName.Length <= Diff)
            StringCbCopyW((PWSTR)((PBYTE)Properties + LogFileNameOffset), Diff, LogFileName);
    }

    // Cleanup
    if (WmiLogInfo)
        RtlFreeHeap(HeapHandle, 0, WmiLogInfo);
    return LastError;
}

ULONG
WINAPI
XYZenumerateTraceGuids(PTRACE_GUID_PROPERTIES* GuidPropertiesArray,
                       ULONG PropertyArrayCount,
                       PULONG GuidCount)
{
    ULONG ReturnLength = 0, ErrorCode = 0, guidCount = 0, i = 0;
    HANDLE HeapHandle = GetProcessHeap();
    PTRACE_GUID_PROPERTIES OutBuffer = NULL, pTemp = NULL;

    if (GuidPropertiesArray && *GuidPropertiesArray)
    {
        if (sizeof (TRACE_GUID_PROPERTIES) * PropertyArrayCount >= INFINITE)
        {
            RtlSetLastWin32Error(ERROR_ARITHMETIC_OVERFLOW);
            return ERROR_ARITHMETIC_OVERFLOW;
        }
        else
        {
            ULONG OutBufferLen = sizeof (TRACE_GUID_PROPERTIES) * PropertyArrayCount;
            OutBuffer = RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, OutBufferLen);

            if (OutBuffer)
            {
                NTSTATUS Status;
                Status = NtTraceControl(TraceControlEnumerateTraceGuids,
                                        NULL,
                                        0,
                                        OutBuffer,
                                        OutBufferLen,
                                        &ReturnLength);
                pTemp = OutBuffer;

                if (Status)
                    ErrorCode = RtlNtStatusToDosError(Status);
                if (Status == STATUS_BUFFER_TOO_SMALL)
                    ErrorCode = ERROR_MORE_DATA;
                else
                    ErrorCode = 0;

                if (!ErrorCode || ErrorCode == ERROR_MORE_DATA)
                {
                    guidCount = ReturnLength / sizeof(TRACE_GUID_PROPERTIES);
                    *GuidCount = guidCount;

                    // If some glitch occurs
                    if (guidCount > PropertyArrayCount)
                        guidCount = 0;

                    // Return every array to caller
                    while (i < guidCount)
                    {
                        GuidPropertiesArray[i]->Guid = pTemp->Guid;
                        GuidPropertiesArray[i]->GuidType = pTemp->GuidType;
                        GuidPropertiesArray[i]->LoggerId = pTemp->LoggerId;
                        GuidPropertiesArray[i]->EnableLevel = pTemp->EnableLevel;
                        GuidPropertiesArray[i]->EnableFlags = pTemp->EnableFlags;
                        GuidPropertiesArray[i]->IsEnable = pTemp->IsEnable;

                        ++pTemp;
                        ++i;
                    }
                }

                // Cleanup
                if (OutBuffer)
                    RtlFreeHeap(HeapHandle, 0, OutBuffer);
                if (ErrorCode)
                    RtlSetLastWin32Error(ErrorCode);
            }
            else
                ErrorCode = GetLastError();
        }
    }
    else
    {
        RtlSetLastWin32Error(ERROR_INVALID_PARAMETER);
        ErrorCode = ERROR_INVALID_PARAMETER;
    }
    return ErrorCode;
}

ULONG
WINAPI
XYZenumerateTraceGuidsEx(TRACE_QUERY_INFO_CLASS TraceQueryInfoClass,
                         PVOID InBuffer,
                         ULONG InBufferSize,
                         PVOID OutBuffer,
                         ULONG OutBufferSize,
                         PULONG ReturnLength)
{
    NTSTATUS Status;
    ULONG ErrorCode, FunctionCode, returnLength;

    if (!ReturnLength)
    {
        RtlSetLastWin32Error(ERROR_INVALID_PARAMETER);
        return ERROR_INVALID_PARAMETER;
    }

    switch (TraceQueryInfoClass)
    {
    case TraceGuidQueryList:
        FunctionCode = TraceControlGetTraceGuidList;
        break;

    case TraceGuidQueryInfo:
        FunctionCode = TraceControlGetTraceGuidInfo;
        break;

    case TraceGroupQueryList:
        FunctionCode = TraceControlGetTraceGroupList;
        break;

    case TraceGroupQueryInfo:
        FunctionCode = TraceControlGetTraceGroupInfo;
        break;

    case TraceMaxLoggersQuery:
        FunctionCode = TraceControlMaxLoggers;
        break;

    default:
        FunctionCode = INFINITE;
    }

    Status = NtTraceControl(FunctionCode,
                            InBuffer,
                            InBufferSize,
                            OutBuffer,
                            OutBufferSize,
                            &returnLength);

    // Return length to caller
    *ReturnLength = returnLength;

    if (Status)
        ErrorCode = RtlNtStatusToDosError(Status);
    else
        ErrorCode = 0;
    return ErrorCode;
}
