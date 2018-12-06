#ifndef WININTERNAL_H
#define WININTERNAL_H

#include <Windows.h>
#include <evntrace.h>

#define EVENT_TRACE_CLOCK_RAW 0
#define EVENT_TRACE_CLOCK_PERFCOUNTER 1
#define EVENT_TRACE_CLOCK_SYSTEMTIME 2
#define EVENT_TRACE_CLOCK_CPUCYCLE 3

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _UNICODE_STRING64 {
    USHORT Length;
    USHORT MaximumLength;
    ULONGLONG Buffer;
} UNICODE_STRING64, *PUNICODE_STRING64;

/* From ProcessHacker phnt/include/ntexapi.h */
typedef enum _EVENT_TRACE_INFORMATION_CLASS {
    EventTraceKernelVersionInformation,
    EventTraceGroupMaskInformation,
    EventTracePerformanceInformation,
    EventTraceTimeProfileInformation,
    EventTraceSessionSecurityInformation,
    EventTraceSpinlockInformation,
    EventTraceStackTracingInformation,
    EventTraceExecutiveResourceInformation,
    EventTraceHeapTracingInformation,
    EventTraceHeapSummaryTracingInformation,
    EventTracePoolTagFilterInformation,
    EventTracePebsTracingInformation,
    EventTraceProfileConfigInformation,
    EventTraceProfileSourceListInformation, 
    EventTraceProfileEventListInformation,
    EventTraceProfileCounterListInformation,
    EventTraceStackCachingInformation,
    EventTraceObjectTypeFilterInformation,
    EventTraceSoftRestartInformation,
    EventTraceLastBranchConfigurationInformation,
    EventTraceLastBranchEventListInformation,
    EventTraceProfileSourceAddInformation,
    EventTraceProfileSourceRemoveInformation,
    EventTraceProcessorTraceConfigurationInformation,
    EventTraceProcessorTraceEventListInformation,
    EventTraceCoverageSamplerInformation,
    MaxEventTraceInfoClass
} EVENT_TRACE_INFORMATION_CLASS;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemPerformanceTraceInformation = 31
} SYSTEM_INFORMATION_CLASS;

typedef struct _EVENT_TRACE_SESSION_SECURITY_INFORMATION {
    EVENT_TRACE_INFORMATION_CLASS EventTraceInformationClass;
    ULONG SecurityInformation;
    ULONG64 TraceHandle;
    SECURITY_DESCRIPTOR SecurityDescriptor[1];
} EVENT_TRACE_SESSION_SECURITY_INFORMATION, *PEVENT_TRACE_SESSION_SECURITY_INFORMATION;

NTSTATUS NtQuerySystemInformation(
    _In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
    _In_ PVOID SystemInformation,
    _In_ ULONG SystemInformationLength,
    _Out_ PULONG ReturnLength);

void RtlSetLastWin32Error(
    _In_ ULONG LastError);

ULONG RtlNtStatusToDosError(
    _In_ NTSTATUS Status);

#endif // WININTERNAL_H
