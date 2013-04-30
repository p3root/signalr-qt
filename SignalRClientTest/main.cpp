#include <QCoreApplication>
#include "Client.h"
#include <QsLog.h>
#include <QsLogDest.h>
#include <QThread>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QsLogging::Logger& logger = QsLogging::Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    QsLogging::DestinationPtr debugDestination(QsLogging::DestinationFactory::MakeDebugOutputDestination() );
    logger.addDestination(debugDestination.get());

    QLOG_DEBUG() << "MainThread: " << a.thread()->currentThreadId();

    Client *c = new Client();
    c->start();



    return a.exec();
}
