#ifndef TRACEVENT_H
#define TRACEVENT_H

#include "CallBacks.h"
#include "PrintProperties.h"

ULONG StartSession(PWCHAR LoggerName, GUID ProviderID);
ULONG ConsumeEvent(PWCHAR LoggerName);
ULONG StopSession(PWCHAR LoggerName);

#endif //TRACEVENT_H
