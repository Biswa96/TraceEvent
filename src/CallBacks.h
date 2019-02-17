#ifndef CALLBACKS_H
#define CALLBACKS_H

void
WINAPI
EventRecordCallback(PEVENT_RECORD eRecord);

ULONG
WINAPI
BufferCallback(PEVENT_TRACE_LOGFILEW Buffer);

#endif //CALLBACKS_H
