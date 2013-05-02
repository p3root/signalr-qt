#include "ConnectionListener.h"
#include "HttpRequest.h"

ConnectionListener::ConnectionListener(QTcpSocket* socket) : _socket(socket)
{
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void ConnectionListener::onReadyRead()
{
    //TODO: read Http Stream
    QByteArray http = _socket->readAll();

    //\r\n\r\n end of http request stream
    HttpRequest* request = HttpRequest::parse(QString(http));

}
