#ifndef WININTERNAL_H
#define WININTERNAL_H

#include <Windows.h>
#include <evntrace.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (Status >= 0)
#endif

#ifndef STATUS_BUFFER_TOO_SMALL
#define STATUS_BUFFER_TOO_SMALL (NTSTATUS)0xC0000023L
#endif

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
    ULONG64 Buffer;
} UNICODE_STRING64, *PUNICODE_STRING64;

//Extracted from ComBase.pdb symbol file
typedef struct _WMI_LOGGER_INFORMATION {
    WNODE_HEADER Wnode;
    ULONG BufferSize;
    ULONG MinimumBuffers;
    ULONG MaximumBuffers;
    ULONG MaximumFileSize;
    ULONG LogFileMode;
    ULONG FlushTimer;
    ULONG EnableFlags;
    union {
        LONG AgeLimit;
        LONG FlushThreshold;
    };
    union {
        struct {
            ULONG Wow : 1;
            ULONG QpcDeltaTracking : 1;
        };
        ULONG64 V2Options;
    };
    union {
        HANDLE LogFileHandle;
        ULONG64 LogFileHandle64;
    };
    union {
        ULONG NumberOfBuffers;
        ULONG InstanceCount;
    };
    union {
        ULONG FreeBuffers;
        ULONG InstanceId;
    };
    union {
        ULONG EventsLost;
        ULONG NumberOfProcessors;
    };
    ULONG BuffersWritten;
    union {
        ULONG LogBuffersLost;
        ULONG Flags;
    };
    ULONG RealTimeBuffersLost;
    union {
        HANDLE LoggerThreadId;
        ULONG64 LoggerThreadId64;
    };
    union {
        UNICODE_STRING LogFileName;
        UNICODE_STRING64 LogFileName64;
    };
    union {
        UNICODE_STRING LoggerName;
        UNICODE_STRING64 LoggerName64;
    };
    ULONG RealTimeConsumerCount;
    ULONG SequenceNumber;
    union {
        PVOID LoggerExtension;
        ULONG64 LoggerExtension64;
    };
} WMI_LOGGER_INFORMATION, *PWMI_LOGGER_INFORMATION;

//Extracted from ComBase.pdb symbol file
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

// Extracted from ntoskrnl
typedef enum _TRACE_CONTROL_FUNCTION_CLASS {
    TraceControlStartLogger = 1,
    TraceControlStopLogger = 2,
    TraceControlQueryLogger = 3,
    TraceControlUpdateLogger = 4,
    TraceControlFlushLogger = 5,
    TraceControlIncrementLoggerFile = 6,

    TraceControlRealtimeConnect = 11,
    TraceControlWdiDispatchControl = 13,
    TraceControlRealtimeDisconnectConsumerByHandle = 14,

    TraceControlReceiveNotification = 16,
    TraceControlEnableGuid = 17, // TraceControlNotifyGuid
    TraceControlSendReplyDataBlock = 18,
    TraceControlReceiveReplyDataBlock = 19,
    TraceControlWdiUpdateSem = 20,
    TraceControlGetTraceGuidList = 21,
    TraceControlGetTraceGuidInfo = 22,
    TraceControlEnumerateTraceGuids = 23,
    // 24
    TraceControlQueryReferenceTime = 25,
    TraceControlTrackProviderBinary = 26,
    TraceControlAddNotificationEvent = 27,
    TraceControlUpdateDisallowList = 28,

    TraceControlUseDescriptorTypeUm = 31,
    TraceControlGetTraceGroupList = 32,
    TraceControlGetTraceGroupInfo = 33,
    TraceControlTraceSetDisallowList= 34,
    TraceControlSetCompressionSettings = 35,
    TraceControlGetCompressionSettings= 36,
    TraceControlUpdatePeriodicCaptureState = 37,
    TraceControlGetPrivateSessionTraceHandle = 38,
    TraceControlRegisterPrivateSession = 39,
    TraceControlQuerySessionDemuxObject = 40,
    TraceControlSetProviderBinaryTracking = 41,
    TraceControlMaxLoggers = 42,
    TraceControlMaxPmcCounter = 43
} TRACE_CONTROL_FUNCTION_CLASS;

NTSTATUS
NTAPI
NtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,
                         PVOID SystemInformation,
                         ULONG SystemInformationLength,
                         PULONG ReturnLength);

// From ProcessHacker phnt/include/ntmisc.h
NTSTATUS
NTAPI
NtTraceControl(TRACE_CONTROL_FUNCTION_CLASS FunctionCode,
               PVOID InBuffer,
               ULONG InBufferLen,
               PVOID OutBuffer,
               ULONG OutBufferLen,
               PULONG ReturnLength);

PVOID
NTAPI
RtlAllocateHeap(PVOID HeapHandle,
                ULONG Flags,
                SIZE_T Size);

BOOLEAN
NTAPI
RtlFreeHeap(HANDLE HeapHandle,
            ULONG Flags,
            PVOID P);

PVOID
NTAPI
RtlReAllocateHeap(HANDLE Heap,
                  ULONG Flags,
                  PVOID Ptr,
                  SIZE_T Size);

void
NTAPI
RtlSetLastWin32Error(ULONG LastError);

ULONG
NTAPI
RtlNtStatusToDosError(NTSTATUS Status);

#undef RtlZeroMemory
void
NTAPI
RtlZeroMemory(PVOID Destination,
              SIZE_T Length);

#endif // WININTERNAL_H
