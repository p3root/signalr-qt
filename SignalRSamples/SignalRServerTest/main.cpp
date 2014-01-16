#include <QCoreApplication>
#include "SignalRServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    SignalRServer s;
    s.start(8080);

    return a.exec();
}
