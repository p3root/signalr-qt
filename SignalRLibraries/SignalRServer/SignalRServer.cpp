#include "SignalRServer.h"
#include <QHttpRequest.h>
#include <QUrl>
#include <QHttpResponse.h>

#include "PersistentConnection.h"

SignalRServer::SignalRServer() : _httpServer(0)
{
    _connection = new PersistentConnection();
}

SignalRServer::SignalRServer(PersistentConnection *connection) : _httpServer(0)
{
    Q_ASSERT(connection);
    _connection = connection;
}

void SignalRServer::start(qint16 port)
{
    if(_httpServer)
        delete _httpServer;

    _httpServer = new QHttpServer();
    _httpServer->listen(port);
    connect(_httpServer, SIGNAL(newRequest(QHttpRequest*,QHttpResponse*)), this, SLOT(newConnection(QHttpRequest*,QHttpResponse*)));
}

void SignalRServer::newConnection(QHttpRequest *req,QHttpResponse *res)
{
    Q_UNUSED(res);

    _connection->processRequest(*req, *res);

    connect(req, SIGNAL(end()), req, SLOT(deleteLater()));
    connect(res, SIGNAL(done()), res, SLOT(deleteLater()));
}
