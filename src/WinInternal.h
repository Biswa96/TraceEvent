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

typedef struct _WMI_LOGGER_INFORMATION {
    WNODE_HEADER Wnode;
    ULONG BufferSize;
    ULONG MinimumBuffers;
    ULONG MaximumBuffers;
    ULONG MaximumFileSize;
    ULONG LogFileMode;
    ULONG FlushTimer;
    ULONG EnableFlags;
    LONG AgeLimit;
    ULONG Wow;
    HANDLE LogFileHandle;
    ULONG NumberOfBuffers;
    ULONG FreeBuffers;
    ULONG EventsLost;
    ULONG BuffersWritten;
    ULONG LogBuffersLost;
    ULONG RealTimeBuffersLost;
    HANDLE LoggerThreadId;
    UNICODE_STRING LogFileName;
    UNICODE_STRING LoggerName;
    PVOID Checksum;
    PVOID LoggerExtension;
} WMI_LOGGER_INFORMATION, *PWMI_LOGGER_INFORMATION;

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

typedef enum _TRACE_CONTROL_FUNCTION_CLASS {
    TraceControlStartLogger = 1,
    TraceControlStopLogger,
    TraceControlQueryLogger,
    TraceControlUpdateLogger,
    TraceControlFlushLogger
} TRACE_CONTROL_FUNCTION_CLASS;

NTSTATUS NtTraceControl(
    _In_ TRACE_CONTROL_FUNCTION_CLASS FunctionCode,
    _In_ PVOID InBuffer,
    _In_ ULONG InBufferLen,
    _Out_ PVOID OutBuffer,
    _In_ ULONG OutBufferLen,
    _Out_ PULONG ReturnLength);

#endif // WININTERNAL_H
