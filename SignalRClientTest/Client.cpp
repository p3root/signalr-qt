#include "Client.h"

#include <Connection.h>
#include <Hubs/HubConnection.h>
#include <MyConnectionHandler.h>
#include <Transports/DefaultHttpClient.h>
#include <Transports/LongPollingTransport.h>
#include <Transports/ServerSentEventsTransport.h>
#include <Helper/Helper.h>

Client::Client()
{
}

void Client::start()
{
    QLOG_DEBUG() << "Client Thread: " << thread()->currentThreadId();
    MyConnectionHandler* handler = new MyConnectionHandler();
    HubConnection* connection = new HubConnection("http://192.168.0.231:8080/signalr", handler);

    HttpClient* client = new DefaultHttpClient();
    ClientTransport* tansport = new LongPollingTransport(client);

    HubProxy* proxy = connection->createHubProxy("Chat");
    connection->start(tansport, true);

    handler->setHubProxy(proxy);
    handler->setConnection(connection);

    Helper::wait(1);

    proxy->invoke("send", "0");
}
