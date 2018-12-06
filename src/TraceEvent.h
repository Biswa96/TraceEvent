#ifndef TRACEVENT_H
#define TRACEVENT_H

#include <wchar.h>

unsigned long StartSession(
    wchar_t* LoggerName,
    struct _GUID* ProviderID);

unsigned long ConsumeEvent(
    wchar_t* LoggerName);

unsigned long StopSession(
    wchar_t* LoggerName);

unsigned long QuerySession(
    wchar_t* LoggerName);

void ListSessions(
    void);

#endif //TRACEVENT_H
