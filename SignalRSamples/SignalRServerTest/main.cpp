#include <QCoreApplication>
#include "SignalRServer.h"

using namespace P3::SignalR::Server;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    SignalRServer s;
    s.start(8080);

    return a.exec();
}
