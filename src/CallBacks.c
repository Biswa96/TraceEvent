#include "CallBacks.h"
#include <stdio.h>

void RemoveTrailingSpace(PEVENT_MAP_INFO mapInfo)
{
    size_t ByteLength = 0;

    for (DWORD i = 0; i < mapInfo->EntryCount; i++)
    {
        ByteLength = (wcslen((PWCHAR)((PBYTE)mapInfo + mapInfo->MapEntryArray[i].OutputOffset)) - 1) * 2;
        *((PWCHAR)((PBYTE)mapInfo + (mapInfo->MapEntryArray[i].OutputOffset + ByteLength))) = L'\0';
    }
}

void GetMapInfo(
    _In_ PEVENT_RECORD eRecord,
    _In_ PWCHAR MapName,
    _In_ ULONG DecodingSource,
    _Out_ PEVENT_MAP_INFO mapInfo)
{
    ULONG result, size = 0;

    result = TdhGetEventMapInformation(eRecord, MapName, mapInfo, &size);
    mapInfo = (PEVENT_MAP_INFO)malloc(size);
    result = TdhGetEventMapInformation(eRecord, MapName, mapInfo, &size);

    if (result == 0)
    {
        if (DecodingSource == DecodingSourceXMLFile)
            RemoveTrailingSpace(mapInfo);
    }
}

//Event Providers use EventWriteTransfer function
void EventRecordCallback(PEVENT_RECORD eRecord)
{
    ULONG result, size = 0;
    PTRACE_EVENT_INFO eInfo = NULL;
    PEVENT_MAP_INFO mapInfo = NULL;

    PBYTE EndOfUserData = (PBYTE)eRecord->UserData + eRecord->UserDataLength;
    PBYTE UserData = (PBYTE)eRecord->UserData;
    ULONG FormattedDataSize = 0;
    PWCHAR FormattedData = NULL;
    USHORT UserDataConsumed = 0;

    //Verbose Event Header Information
    FILETIME ft;
    SYSTEMTIME st;

    ft.dwHighDateTime = eRecord->EventHeader.TimeStamp.HighPart;
    ft.dwLowDateTime = eRecord->EventHeader.TimeStamp.LowPart;
    FileTimeToSystemTime(&ft, &st);

    wprintf(L"%02d/%02d/%04d-%02d:%02d:%02d.%03d :: "
        L"ThreadID: %ld ProcessID: %ld\n",
        st.wMonth, st.wDay, st.wYear,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        eRecord->EventHeader.ThreadId,
        eRecord->EventHeader.ProcessId);

    //Get Event Information
    result = TdhGetEventInformation(eRecord, 0, NULL, NULL, &size);
    eInfo = (PTRACE_EVENT_INFO)malloc(size);
    result = TdhGetEventInformation(eRecord, 0, NULL, eInfo, &size);
    if (result != 0)
    {
        wprintf(L"TdhGetEventInformation Error: %ld\n", result);
        return;
    }

    //Print Event Information: Provider Name and Task Name
    if (eInfo->ProviderNameOffset > 0)
        wprintf(L"[%ls] ", (PWCHAR)((PBYTE)(eInfo) + eInfo->ProviderNameOffset));
    if (eInfo->TaskNameOffset > 0)
        wprintf(L"%ls ", (PWCHAR)((PBYTE)(eInfo) + eInfo->TaskNameOffset));

    //Print Event Property: Property Name and Property Data
    if (eInfo->TopLevelPropertyCount > 0)
    {
        for (ULONG i = 0; i < eInfo->TopLevelPropertyCount; i++)
        {
            wprintf(L"%ls:",
                (PWCHAR)((PBYTE)(eInfo) + eInfo->EventPropertyInfoArray[i].NameOffset));

            GetMapInfo(
                eRecord,
                (PWCHAR)((PBYTE)(eInfo) + eInfo->EventPropertyInfoArray[i].nonStructType.MapNameOffset),
                eInfo->DecodingSource,
                mapInfo);

            result = TdhFormatProperty(
                eInfo,
                mapInfo,
                sizeof(PVOID),
                eInfo->EventPropertyInfoArray[i].nonStructType.InType,
                eInfo->EventPropertyInfoArray[i].nonStructType.OutType,
                eInfo->EventPropertyInfoArray[i].length,
                (USHORT)(EndOfUserData - UserData),
                UserData,
                &FormattedDataSize,
                FormattedData,
                &UserDataConsumed);

            FormattedData = (PWCHAR)malloc(FormattedDataSize);

            result = TdhFormatProperty(
                eInfo,
                mapInfo,
                sizeof(PVOID),
                eInfo->EventPropertyInfoArray[i].nonStructType.InType,
                eInfo->EventPropertyInfoArray[i].nonStructType.OutType,
                eInfo->EventPropertyInfoArray[i].length,
                (USHORT)(EndOfUserData - UserData),
                UserData,
                &FormattedDataSize,
                FormattedData,
                &UserDataConsumed);

            if (result == 0)
            {
                wprintf(L"%ls ", FormattedData);
                UserData += UserDataConsumed;
            }
        }
    }

    //Cleanup
    wprintf(L"\n");
    free(eInfo);
    free(mapInfo);
    free(FormattedData);
}

//Print Buffer Statistics after every Event Buffer flushes
ULONG TotalLost = 0;

ULONG BufferCallback(PEVENT_TRACE_LOGFILEW Buffer)
{
    TotalLost += Buffer->EventsLost;

    wprintf(
        L"Filled=%8d, Lost=%3d TotalLost= %d\r",
        Buffer->Filled, Buffer->EventsLost, TotalLost);

    return TRUE;
}
