#ifndef HELPERS_H
#define HELPERS_H

ULONG
WINAPI
EtwpValidateTraceProperties(PEVENT_TRACE_PROPERTIES_V2 Properties,
                            PULONG pFilterDescCount,
                            PEVENT_FILTER_DESCRIPTOR* pFilterDesc,
                            PULONG ReturnedLength);

void
WINAPI
EtwpCopyPropertiesToInfo(PEVENT_TRACE_PROPERTIES_V2 Properties,
                         PWMI_LOGGER_INFORMATION WmiLogInfo);

void
WINAPI
EtwpCopyInfoToProperties(PWMI_LOGGER_INFORMATION WmiLogInfo,
                         PEVENT_TRACE_PROPERTIES_V2 Properties);

#endif // HELPERS_H
