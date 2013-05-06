#include "AutoTransport.h"
#include "ServerSentEventsTransport.h"
#include "LongPollingTransport.h"
#include "Helper/TransportHelper.h"

AutoTransport::AutoTransport(HttpClient* client)
{
    _httpClient = client;


    _transports.append(new ServerSentEventsTransport(client));
    _transports.append(new LongPollingTransport(client));


}

AutoTransport::AutoTransport(HttpClient *client, QList<ClientTransport *>transports)
{
    _httpClient = client;
    _transports = transports;
}

AutoTransport::~AutoTransport()
{
    foreach(ClientTransport* cur, _transports)
    {
        delete cur;
    }
}

void AutoTransport::negotiate(Connection *connection, ClientTransport::NEGOTIATE_CALLBACK negotiateCallback, void *state)
{
    TransportHelper::getNegotiationResponse(_httpClient, connection, negotiateCallback, state);
}

void AutoTransport::start(Connection *connection, ClientTransport::START_CALLBACK startCallback, QString data, void *state)
{
    _transports[0]->start(connection, startCallback, data, state);
}

void AutoTransport::send(Connection *connection, QString data)
{
    _transports[0]->send(connection, data);
}

void AutoTransport::stop(Connection *connection)
{
     _transports[0]->stop(connection);
}

void AutoTransport::abort(Connection *connection)
{
    _transports[0]->abort(connection);
}
