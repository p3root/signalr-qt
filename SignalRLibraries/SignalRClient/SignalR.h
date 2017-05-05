#ifndef SIGNALR_H
#define SIGNALR_H

#include "SignalR_global.h"

namespace P3 { namespace SignalR { namespace Client {

#if __cplusplus > 199711L
    #define HAS_CPP11
#endif

#ifdef HAS_CPP11
    #define OVERRIDE_M override
#else
    #define OVERRIDE_M
#endif

class SIGNALR_EXPORT SignalR
{
public:
    enum State
    {
        Connecting,
        Connected,
        Reconnecting,
        Disconnecting,
        Disconnected
    };

    enum LogSeverity
    {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Special
    };
};

}}}
#endif // SIGNALR_H
