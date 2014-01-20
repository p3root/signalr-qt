#ifndef ICONNECTION_H
#define ICONNECTION_H

#include <QString>
#include "ConnectionMessage.h"

class IConnection
{
public:
    virtual const QString &getDefaultSignal() = 0;
    virtual void send(const ConnectionMessage &message) = 0;
};

#endif // ICONNECTION_H
