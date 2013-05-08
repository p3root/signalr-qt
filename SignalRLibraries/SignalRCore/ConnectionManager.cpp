#include "ConnectionManager.h"
#include <QEventLoop>

ConnectionManager::ConnectionManager(SignalRServer &server) : _server(server)
{
    _connection = new QTcpServer();
}

void ConnectionManager::startListener()
{
    _connection->listen(QHostAddress::Any, _server.getPort());
    connect(_connection, SIGNAL(newConnection()), this, SLOT(acceptConnection()));

    start();
}

void ConnectionManager::stop()
{
    _isRunning = false;
    quit();
}

void ConnectionManager::run()
{
    _isRunning = true;


    exec();

    _isRunning = false;
}

void ConnectionManager::acceptConnection()
{
    QTcpSocket* sock = _connection->nextPendingConnection();
    ConnectionListener *listener = new ConnectionListener(sock);
    connect(listener, SIGNAL(onDelete(ConnectionListener*)), this, SLOT(onDeleteListener(ConnectionListener*)));

    _listeners.append(listener);
}

void ConnectionManager::onDeleteListener(ConnectionListener *list)
{
    _listeners.removeOne(list);
    delete list;
}
