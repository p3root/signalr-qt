#include "Client.h"

#include <QsLog.h>
#include <QsLogDest.h>

#include <Connection.h>
#include <Hubs/HubConnection.h>
#include <MyConnectionHandler.h>
#include <Transports/DefaultHttpClient.h>
#include <Transports/LongPollingTransport.h>
#include <Transports/ServerSentEventsTransport.h>
#include <Helper/Helper.h>

Client::Client()
{
    qDebug() << "Started";
    QLOG_DEBUG() << "";
    QsLogging::Logger& logger = QsLogging::Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    QsLogging::DestinationPtr debugDestination(QsLogging::DestinationFactory::MakeDebugOutputDestination() );
    logger.addDestination(debugDestination.get());
}

void Client::start()
{
    QLOG_DEBUG() << "Client Thread: " << thread()->currentThreadId();
    MyConnectionHandler* handler = new MyConnectionHandler();
    HubConnection* connection = new HubConnection("http://localhost:8080/signalr", handler);

    HttpClient* client = new DefaultHttpClient();
    ClientTransport* tansport = new LongPollingTransport(client);

    HubProxy* proxy = connection->createHubProxy("Chat");
    connection->start(tansport, true);

    handler->setHubProxy(proxy);
    handler->setConnection(connection);

    Helper::wait(1);

    proxy->invoke("send", "0");
}
