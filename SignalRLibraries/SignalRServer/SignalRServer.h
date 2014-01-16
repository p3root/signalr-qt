#ifndef SIGNALRSERVER_H
#define SIGNALRSERVER_H

#include <QObject>
#include <QHostAddress>
#include "signalrserver_global.h"
#include <QHttpServer.h>

class PersistentConnection;

class SIGNALRSERVERSHARED_EXPORT SignalRServer : public QObject
{
    Q_OBJECT
    
public:
    SignalRServer();
    SignalRServer(PersistentConnection *connection);
    void start(qint16 port);

private Q_SLOTS:
    void newConnection(QHttpRequest *req, QHttpResponse *res);

private:
    QHttpServer *_httpServer;
    PersistentConnection *_connection;
};

#endif // SIGNALRSERVER_H
