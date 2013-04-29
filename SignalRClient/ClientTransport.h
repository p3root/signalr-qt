#ifndef CLIENTTRANSPORT_H
#define CLIENTTRANSPORT_H

class Connection;

#include <QString>
#include "SignalException.h"
#include "Transports/NegotiateResponse.h"

class ClientTransport
{
public:
    ClientTransport(void);
    virtual ~ClientTransport(void);

    typedef void (*START_CALLBACK)(SignalException* error, void* state);
    typedef void (*NEGOTIATE_CALLBACK)(NegotiateResponse* negotiateResponse, SignalException* error, void* state);

    virtual void negotiate(Connection* connection, NEGOTIATE_CALLBACK negotiateCallback, void* state = NULL) = 0;
    virtual void start(Connection* connection, START_CALLBACK startCallback, QString data, void* state = NULL) = 0;
    virtual void send(Connection* connection, QString data) = 0;
    virtual void stop(Connection* connection) = 0;
    virtual void abort(Connection* connection) = 0;
};

#endif
