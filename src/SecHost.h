#ifndef SECHOST_H
#define SECHOST_H

ULONG
WINAPI
EtwpCacheMaxLogger(PULONG EtwpMaxLoggers);

ULONG
WINAPI
XYZstartTraceW(PTRACEHANDLE TraceHandle,
               PWSTR InstanceName,
               PEVENT_TRACE_PROPERTIES_V2 Properties);

ULONG
WINAPI
XYZcontrolTraceW(TRACEHANDLE TraceHandle,
                 PWSTR InstanceName,
                 PEVENT_TRACE_PROPERTIES_V2 Properties,
                 ULONG ControlCode);

ULONG
WINAPI
XYZenumerateTraceGuids(PTRACE_GUID_PROPERTIES* GuidPropertiesArray,
                       ULONG PropertyArrayCount,
                       PULONG GuidCount);

#endif // SECHOST_H
