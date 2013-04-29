#ifndef AUTOTRANSPORT_H
#define AUTOTRANSPORT_H

#include "ClientTransport.h"
#include "HttpClient.h"
#include <QList>

class AutoTransport : public ClientTransport
{
public:
    AutoTransport(HttpClient*);
    AutoTransport(HttpClient*, QList<ClientTransport*>);
    virtual ~AutoTransport(void);

    void negotiate(Connection* connection, NEGOTIATE_CALLBACK negotiateCallback, void* state);
    void start(Connection* connection, START_CALLBACK startCallback, QString data, void* state);
    void send(Connection* connection, QString data);
    void stop(Connection* connection);
    void abort(Connection *connection);


private:
    ClientTransport* _transport;
    HttpClient* _httpClient;
    QList<ClientTransport*> _transports;

};

#endif // AUTOTRANSPORT_H
