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

ULONG X_StartTraceW(
    PTRACEHANDLE TraceHandle,
    PWSTR InstanceName,
    PEVENT_TRACE_PROPERTIES_V2 Properties)
{
    ULONG FilterDescCount, ReturnedLength, LastError;
    ULONG InstnaceNameLength = -1, LogFileNameLength = -1, BufferSize = 0;
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

    LastError = EtwpValidateTraceProperties(
        Properties,
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
        ++InstnaceNameLength;
    } while (InstanceName[InstnaceNameLength]);
    BufferSize = ReturnedLength + (sizeof(wchar_t) * (InstnaceNameLength + 1));

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

        BufferSize += (sizeof(wchar_t) * (LogFileNameLength + 1));
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
    PWMI_LOGGER_INFORMATION WmiLogInfo = malloc(Properties->Wnode.BufferSize);
    memset(WmiLogInfo, 0, Properties->Wnode.BufferSize);
    EtwpCopyPropertiesToInfo(Properties, WmiLogInfo);

    WmiLogInfo->LoggerName.Length = (USHORT)(sizeof(wchar_t) * InstnaceNameLength);
    WmiLogInfo->LoggerName.MaximumLength = (USHORT)(sizeof(wchar_t) * (InstnaceNameLength + 1));
    WmiLogInfo->LoggerName.Buffer = InstanceName;

    if (IsLogFilePresent)
    {
        WmiLogInfo->LogFileName.Length = (USHORT)(sizeof(wchar_t) * LogFileNameLength);
        WmiLogInfo->LogFileName.MaximumLength = (USHORT)(sizeof(wchar_t) * (LogFileNameLength + 1));
        WmiLogInfo->LogFileName.Buffer = LogFileName;
    }

    NTSTATUS Status = NtTraceControl(
        TraceControlStartLogger,
        WmiLogInfo,
        WmiLogInfo->Wnode.BufferSize,
        WmiLogInfo,
        WmiLogInfo->Wnode.BufferSize,
        &ReturnedLength);

    LastError = RtlNtStatusToDosError(Status);
    if (!LastError)
    {
        *TraceHandle = WmiLogInfo->Wnode.HistoricalContext;
        EtwpCopyInfoToProperties(WmiLogInfo, Properties);

        ULONG LoggerNameOffset = Properties->LoggerNameOffset;
        ULONG LogFileNameOffset = Properties->LogFileNameOffset;
        if (LoggerNameOffset > LogFileNameOffset)
            LogFileNameOffset = Properties->Wnode.BufferSize;
        ULONG Diff = LogFileNameOffset - LoggerNameOffset;

        if (sizeof(wchar_t) * (InstnaceNameLength + 1) <= Diff)
            StringCbCopyW((PWSTR)((PBYTE)Properties + LoggerNameOffset), Diff, InstanceName);

        if (LogFileNameOffset > LoggerNameOffset)
            LoggerNameOffset = Properties->Wnode.BufferSize;
        Diff = LoggerNameOffset - LogFileNameOffset;
        if (WmiLogInfo->LogFileName.Length && WmiLogInfo->LogFileName.Length <=Diff)
            StringCbCopyW((PWSTR)((PBYTE)Properties + LogFileNameOffset), Diff, InstanceName);
    }

    free(WmiLogInfo);
    return LastError;
}
