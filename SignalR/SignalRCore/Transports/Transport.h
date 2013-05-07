#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "Http/HttpResponse.h"
#include "Http/HttpRequest.h"

class Transport
{
public:
    Transport();

    virtual void connected() = 0;
    virtual void reconnected() = 0;
    virtual void disconnected() = 0;

    virtual HttpResponse* processRequest(HttpRequest*)=0;
    virtual void send(QString data)=0;

protected:
    QString _connectionId;
};

#endif // TRANSPORT_H
