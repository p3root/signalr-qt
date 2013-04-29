#include <QCoreApplication>
#include <Connection.h>
#include <Hubs/HubConnection.h>
#include <MyConnectionHandler.h>
#include <Transports/DefaultHttpClient.h>
#include <QsLog.h>
#include <QsLogDest.h>
#include <unistd.h>
#include <Transports/LongPollingTransport.h>
#include <Transports/ServerSentEventsTransport.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QsLogging::Logger& logger = QsLogging::Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    QsLogging::DestinationPtr debugDestination(QsLogging::DestinationFactory::MakeDebugOutputDestination() );
    logger.addDestination(debugDestination.get());


    ConnectionHandler* handler = new MyConnectionHandler();
    HttpClient* client = new DefaultHttpClient();
    ClientTransport* tansport = new LongPollingTransport(client);

    HubConnection connection = HubConnection("http://192.168.0.231:8080/signalr", handler);
    HubProxy* proxy = connection.createHubProxy("Chat");
    connection.start(tansport);

    sleep(2);


    proxy->invoke("send", "c++: test123");

    sleep(1);
    proxy->invoke("send", "c++: asdf");
    proxy->invoke("send", "c++: testasdf");
    proxy->invoke("send", "c++: asdfx3");
    proxy->invoke("send", "c++: asdfxcv23");
    proxy->invoke("send", "c++: asdfxv");
    sleep(10);




    return a.exec();
}
