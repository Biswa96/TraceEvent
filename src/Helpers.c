#include "WinInternal.h"

ULONG
WINAPI
EtwpValidateTraceProperties(PEVENT_TRACE_PROPERTIES_V2 Properties,
                            PULONG pFilterDescCount,
                            PEVENT_FILTER_DESCRIPTOR* pFilterDesc,
                            PULONG ReturnedLength)
{
    if (Properties->Wnode.BufferSize < sizeof(EVENT_TRACE_PROPERTIES))
        return ERROR_BAD_LENGTH;

    ULONG LoggerNameOffset = Properties->LoggerNameOffset;
    if (LoggerNameOffset && (LoggerNameOffset < sizeof(EVENT_TRACE_PROPERTIES) ||
        LoggerNameOffset > Properties->Wnode.BufferSize))
        return ERROR_INVALID_PARAMETER;

    ULONG LogFileNameOffset = Properties->LogFileNameOffset;
    if (LogFileNameOffset)
    {
        if (LogFileNameOffset < sizeof(EVENT_TRACE_PROPERTIES) ||
            LogFileNameOffset > Properties->Wnode.BufferSize)
            return ERROR_INVALID_PARAMETER;
    }

    if (!(Properties->Wnode.Flags & WNODE_FLAG_VERSIONED_PROPERTIES))
    {
        *ReturnedLength = sizeof(EVENT_TRACE_PROPERTIES);
        return ERROR_SUCCESS;
    }

    *ReturnedLength = sizeof(EVENT_TRACE_PROPERTIES_V2);
    if (Properties->Wnode.BufferSize < sizeof(EVENT_TRACE_PROPERTIES_V2))
        return ERROR_BAD_LENGTH;

    PEVENT_FILTER_DESCRIPTOR FilterDesc = Properties->FilterDesc;
    ULONG FilterDescCount = Properties->FilterDescCount;
    if ((FilterDescCount == 0) != (FilterDesc == NULL))
        return ERROR_INVALID_PARAMETER;

    if (!FilterDescCount ||
        !(Properties->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE))
        return ERROR_SUCCESS;
    // EtwpValidateFilterDescriptors(TRUE, 0, FilterDescCount, FilterDesc)

    *pFilterDescCount = FilterDescCount;
    *pFilterDesc = FilterDesc;
    return ERROR_SUCCESS;
}

void
WINAPI
EtwpCopyPropertiesToInfo(PEVENT_TRACE_PROPERTIES_V2 Properties,
                         PWMI_LOGGER_INFORMATION WmiLogInfo)
{
    WmiLogInfo->Wnode.BufferSize = Properties->Wnode.BufferSize;
    WmiLogInfo->Wnode.HistoricalContext = Properties->Wnode.HistoricalContext;
    WmiLogInfo->Wnode.CountLost = Properties->Wnode.CountLost;

    if (Properties->Wnode.ClientContext)
        WmiLogInfo->Wnode.ClientContext = Properties->Wnode.ClientContext;
    else
        WmiLogInfo->Wnode.ClientContext = EVENT_TRACE_CLOCK_PERFCOUNTER;

    WmiLogInfo->Wnode.Flags = Properties->Wnode.Flags;
    WmiLogInfo->BufferSize = Properties->BufferSize;
    WmiLogInfo->MinimumBuffers = Properties->MinimumBuffers;
    WmiLogInfo->MaximumBuffers = Properties->MaximumBuffers;
    WmiLogInfo->MaximumFileSize = Properties->MaximumFileSize;
    WmiLogInfo->LogFileMode = Properties->LogFileMode;
    WmiLogInfo->FlushTimer = Properties->FlushTimer;
    WmiLogInfo->EnableFlags = Properties->EnableFlags;
    WmiLogInfo->AgeLimit = Properties->AgeLimit;
    WmiLogInfo->NumberOfBuffers = Properties->NumberOfBuffers;
    WmiLogInfo->FreeBuffers = Properties->FreeBuffers;
    WmiLogInfo->EventsLost = Properties->EventsLost;
    WmiLogInfo->BuffersWritten = Properties->BuffersWritten;
    WmiLogInfo->LogBuffersLost = Properties->LogBuffersLost;
    WmiLogInfo->RealTimeBuffersLost = Properties->RealTimeBuffersLost;
    WmiLogInfo->LoggerThreadId = Properties->LoggerThreadId;

    if (WmiLogInfo->Wnode.Flags & WNODE_FLAG_VERSIONED_PROPERTIES)
        WmiLogInfo->V2Options = 0;
    else
    WmiLogInfo->V2Options = Properties->V2Options;
}

#define DISABLE_VERSIONED_PROPERTIES \
    (~(WNODE_FLAG_ALL_DATA | WNODE_FLAG_VERSIONED_PROPERTIES))

void
WINAPI
EtwpCopyInfoToProperties(PWMI_LOGGER_INFORMATION WmiLogInfo,
                         PEVENT_TRACE_PROPERTIES_V2 Properties)
{
    Properties->Wnode.BufferSize = WmiLogInfo->Wnode.BufferSize;
    Properties->Wnode.ProviderId = WmiLogInfo->RealTimeConsumerCount;
    Properties->Wnode.HistoricalContext = WmiLogInfo->Wnode.HistoricalContext;
    Properties->Wnode.CountLost = WmiLogInfo->Wnode.CountLost;
    Properties->Wnode.Guid = WmiLogInfo->Wnode.Guid;
    Properties->Wnode.ClientContext = WmiLogInfo->Wnode.ClientContext;
    Properties->Wnode.Flags = WmiLogInfo->Wnode.Flags;
    Properties->BufferSize = WmiLogInfo->BufferSize;
    Properties->MinimumBuffers = WmiLogInfo->MinimumBuffers;
    Properties->MaximumBuffers = WmiLogInfo->MaximumBuffers;
    Properties->MaximumFileSize = WmiLogInfo->MaximumFileSize;
    Properties->LogFileMode = WmiLogInfo->LogFileMode;
    Properties->FlushTimer = WmiLogInfo->FlushTimer;
    Properties->EnableFlags = WmiLogInfo->EnableFlags;
    Properties->AgeLimit = WmiLogInfo->AgeLimit;
    Properties->NumberOfBuffers = WmiLogInfo->NumberOfBuffers;
    Properties->FreeBuffers = WmiLogInfo->FreeBuffers;
    Properties->EventsLost = WmiLogInfo->EventsLost;
    Properties->BuffersWritten = WmiLogInfo->BuffersWritten;
    Properties->LogBuffersLost = WmiLogInfo->LogBuffersLost;
    Properties->RealTimeBuffersLost = WmiLogInfo->RealTimeBuffersLost;
    Properties->LoggerThreadId = WmiLogInfo->LoggerThreadId;

    if (Properties->Wnode.Flags & WNODE_FLAG_VERSIONED_PROPERTIES)
    {
        Properties->Wnode.Flags |= WNODE_FLAG_VERSIONED_PROPERTIES;
        Properties->V2Options = WmiLogInfo->V2Options;
    }
    else
        Properties->Wnode.Flags &= DISABLE_VERSIONED_PROPERTIES;
}
