#ifndef SECHOST_H
#define SECHOST_H

#include <wchar.h>

typedef unsigned long ULONG, *PULONG;
typedef wchar_t* PWSTR;
typedef unsigned long long ULONG64, TRACEHANDLE, *PTRACEHANDLE;
typedef struct _EVENT_TRACE_PROPERTIES_V2 *PEVENT_TRACE_PROPERTIES_V2;

ULONG X_StartTraceW(
    PTRACEHANDLE TraceHandle,
    PWSTR InstanceName,
    PEVENT_TRACE_PROPERTIES_V2 Properties);

#endif // SECHOST_H
