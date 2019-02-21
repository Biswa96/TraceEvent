#ifndef TRACEVENT_H
#define TRACEVENT_H

ULONG
WINAPI
StartSession(PWSTR LoggerName, GUID* ProviderID);

ULONG
WINAPI
ConsumeEvent(PWSTR LoggerName);

ULONG
WINAPI
StopSession(PWSTR LoggerName);

ULONG
WINAPI
QuerySession(PWSTR LoggerName);

ULONG
WINAPI
ListSessions(void);

void
WINAPI
EnumGuids(void);

void
WINAPI
EnumGuidsInfo(void);

#endif //TRACEVENT_H
