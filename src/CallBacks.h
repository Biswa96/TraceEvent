#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <windows.h>
#include <tdh.h>

void EventRecordCallback(PEVENT_RECORD EventRecord);
ULONG BufferCallback(PEVENT_TRACE_LOGFILEW Buffer);

#endif //CALLBACKS_H
