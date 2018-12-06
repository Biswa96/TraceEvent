#include <windows.h>
#include <tdh.h>
#include <stdio.h>

#define LODWORD(x) ((DWORD)(x))
#define HIDWORD(x) ((DWORD)(((x) >> 32) & 0xffffffff))

void RemoveTrailingSpace(
    PEVENT_MAP_INFO EventMapInfo)
{
    size_t ByteLength = 0;

    for (DWORD i = 0; i < EventMapInfo->EntryCount; i++)
    {
        ByteLength = (wcslen((PWCHAR)((PBYTE)EventMapInfo + EventMapInfo->MapEntryArray[i].OutputOffset)) - 1) * sizeof(wchar_t);
        *((PWCHAR)((PBYTE)EventMapInfo + (EventMapInfo->MapEntryArray[i].OutputOffset + ByteLength))) = L'\0';
    }
}

void GetMapInfo(
    PEVENT_RECORD EventRecord,
    PWCHAR MapName,
    ULONG DecodingSource,
    PEVENT_MAP_INFO EventMapInfo)
{
    ULONG MapSize = 0;
    ULONG result = TdhGetEventMapInformation(EventRecord, MapName, EventMapInfo, &MapSize);
    EventMapInfo = malloc(MapSize);
    result = TdhGetEventMapInformation(EventRecord, MapName, EventMapInfo, &MapSize);

    if (result == ERROR_SUCCESS)
    {
        if (DecodingSource == DecodingSourceXMLFile)
            RemoveTrailingSpace(EventMapInfo);
    }
}

// Event Providers use EventWriteTransfer function
void EventRecordCallback(
    struct _EVENT_RECORD* EventRecord)
{
    PEVENT_MAP_INFO EventMapInfo = NULL;
    PBYTE EndOfUserData = (PBYTE)EventRecord->UserData + EventRecord->UserDataLength;
    PBYTE UserData = (PBYTE)EventRecord->UserData;
    ULONG FormattedDataSize = 0;
    PWCHAR FormattedData = NULL;
    USHORT UserDataConsumed = 0;

    // Verbose Event Header Information
    FILETIME ft;
    SYSTEMTIME st;

    ft.dwHighDateTime = EventRecord->EventHeader.TimeStamp.HighPart;
    ft.dwLowDateTime = EventRecord->EventHeader.TimeStamp.LowPart;
    FileTimeToSystemTime(&ft, &st);

    wprintf(L"%02d/%02d/%04d-%02d:%02d:%02d.%03d :: "
        L"ThreadID: %ld ProcessID: %ld\n",
        st.wMonth, st.wDay, st.wYear,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        EventRecord->EventHeader.ThreadId,
        EventRecord->EventHeader.ProcessId);

    // Get Event Information
    ULONG size = 0;
    ULONG result = TdhGetEventInformation(EventRecord, 0, NULL, NULL, &size);
    PTRACE_EVENT_INFO EventInfo = malloc(size);
    result = TdhGetEventInformation(EventRecord, 0, NULL, EventInfo, &size);
    if (result != ERROR_SUCCESS)
    {
        wprintf(L"TdhGetEventInformation Error: %ld\n", result);
        return;
    }

    // Print Event Information: Provider Name and Task Name
    if (EventInfo->ProviderNameOffset > 0)
        wprintf(L"[%ls] ", (PWCHAR)((PBYTE)(EventInfo) +EventInfo->ProviderNameOffset));
    if (EventInfo->TaskNameOffset > 0)
        wprintf(L"%ls ", (PWCHAR)((PBYTE)(EventInfo) +EventInfo->TaskNameOffset));

    // Print Event Property: Property Name and Property Data
    if (EventInfo->TopLevelPropertyCount > 0)
    {
        for (ULONG i = 0; i < EventInfo->TopLevelPropertyCount; i++)
        {
            wprintf(L"%ls:",
                (PWCHAR)((PBYTE)(EventInfo) +EventInfo->EventPropertyInfoArray[i].NameOffset));

            GetMapInfo(
                EventRecord,
                (PWCHAR)((PBYTE)(EventInfo) +EventInfo->EventPropertyInfoArray[i].nonStructType.MapNameOffset),
                EventInfo->DecodingSource,
                EventMapInfo);

            result = TdhFormatProperty(
                EventInfo,
                EventMapInfo,
                sizeof(PVOID),
                EventInfo->EventPropertyInfoArray[i].nonStructType.InType,
                EventInfo->EventPropertyInfoArray[i].nonStructType.OutType,
                EventInfo->EventPropertyInfoArray[i].length,
                (USHORT)(EndOfUserData - UserData),
                UserData,
                &FormattedDataSize,
                FormattedData,
                &UserDataConsumed);

            FormattedData = malloc(FormattedDataSize);

            result = TdhFormatProperty(
                EventInfo,
                EventMapInfo,
                sizeof(PVOID),
                EventInfo->EventPropertyInfoArray[i].nonStructType.InType,
                EventInfo->EventPropertyInfoArray[i].nonStructType.OutType,
                EventInfo->EventPropertyInfoArray[i].length,
                (USHORT)(EndOfUserData - UserData),
                UserData,
                &FormattedDataSize,
                FormattedData,
                &UserDataConsumed);

            if (result == ERROR_SUCCESS)
            {
                wprintf(L"%ls ", FormattedData);
                UserData += UserDataConsumed;
            }
        }
    }

    // Cleanup
    wprintf(L"\n");
    free(EventInfo);
    free(EventMapInfo);
    free(FormattedData);
}

// Print Buffer Statistics after every Event Buffer flushes
unsigned long TotalLost = 0, BlockNumber = 0;

unsigned long BufferCallback(
    struct _EVENT_TRACE_LOGFILEW* LogFile)
{
    SYSTEMTIME st;
    FileTimeToSystemTime((PFILETIME)&LogFile->CurrentTime, &st);

    // Increment Counting variables
    TotalLost += LogFile->EventsLost;
    ++BlockNumber;

    wprintf(
        L"\n%02d/%02d/%04d-%02d:%02d:%02d.%03d :: %8d: Filled=%8d, Lost=%3d TotalLost= %d\r",
        st.wMonth, st.wDay, st.wYear,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        BlockNumber, LogFile->Filled, LogFile->EventsLost, TotalLost);

    return TRUE;
}
