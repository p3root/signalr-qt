#ifndef SIGNALR_H
#define SIGNALR_H

#include "SignalR_global.h"

namespace P3 { namespace SignalR { namespace Client {

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
        Debug,
        Info,
        Warning,
        Error
    };
};

}}}
#endif // SIGNALR_H
