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
#include <Helper/Helper.h>

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

    Helper::sleep(2000);

    proxy->invoke("send", "c++: test123");

    Helper::sleep(1000);
    proxy->invoke("send", "c++: asdf");
    proxy->invoke("send", "c++: testasdf");
    proxy->invoke("send", "c++: asdfx3");
    proxy->invoke("send", "c++: asdfxcv23");
    proxy->invoke("send", "c++: asdfxv");
    Helper::sleep(10000);




    return a.exec();
}
