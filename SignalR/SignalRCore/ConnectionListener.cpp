#include "ConnectionListener.h"
#include "Http/HttpRequest.h"
#include "ConnectionHelper.h"

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

    HttpResponse* res = ConnectionHelper::processRequest(request);

    _socket->write(res->getHttpResponse());

    delete request;
    delete res;

    _socket->close();
    Q_EMIT onDelete(this);
}

QString ConnectionListener::getHttpRequest(QString str)
{
    if(str.endsWith(HTTP_END_REQUEST))
    {
        return str;
    }

    while(!str.endsWith(HTTP_END_REQUEST))
    {
        str += _socket->read(1);
    }

    return str;
}
