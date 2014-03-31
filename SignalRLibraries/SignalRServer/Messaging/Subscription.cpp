#include "Subscription.h"


namespace P3 { namespace SignalR { namespace Server {

const QString& Subscription::getIdentity()
{
    static QString empty("");
    return empty;
}


}}}
