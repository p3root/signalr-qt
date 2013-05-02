#include <QCoreApplication>
#include <SignalRServer.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    SignalRServer* server = new SignalRServer();
    server->start(8080);

    return a.exec();
}
