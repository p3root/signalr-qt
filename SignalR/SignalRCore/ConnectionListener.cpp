#include "ConnectionListener.h"

ConnectionListener::ConnectionListener(QTcpSocket* socket) : _socket(socket)
{
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void ConnectionListener::onReadyRead()
{
    QByteArray array = _socket->readAll();


    QString test = "test";
}
