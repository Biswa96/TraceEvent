#ifndef HELPERS_H
#define HELPERS_H

typedef unsigned long ULONG, *PULONG;
typedef struct _EVENT_TRACE_PROPERTIES_V2 *PEVENT_TRACE_PROPERTIES_V2;
typedef struct _EVENT_FILTER_DESCRIPTOR *PEVENT_FILTER_DESCRIPTOR;
typedef struct _WMI_LOGGER_INFORMATION *PWMI_LOGGER_INFORMATION;

ULONG EtwpValidateTraceProperties(
    PEVENT_TRACE_PROPERTIES_V2 Properties,
    PULONG pFilterDescCount,
    PEVENT_FILTER_DESCRIPTOR* pFilterDesc,
    PULONG ReturnedLength);

void EtwpCopyPropertiesToInfo(
    PEVENT_TRACE_PROPERTIES_V2 Properties,
    PWMI_LOGGER_INFORMATION WmiLogInfo);

void EtwpCopyInfoToProperties(
    PWMI_LOGGER_INFORMATION WmiLogInfo,
    PEVENT_TRACE_PROPERTIES_V2 Properties);

#endif // HELPERS_H
