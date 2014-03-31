#ifndef ISUBSCRIPTION_H
#define ISUBSCRIPTION_H

#include <QString>

namespace P3 { namespace SignalR { namespace Server {

class Subscription
{
public:
    const virtual QString &getIdentity();
};

}}}

#endif // ISUBSCRIPTION_H
