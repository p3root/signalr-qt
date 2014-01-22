#include "WebSocketTransport.h"

WebSocketTransport::WebSocketTransport(HttpClient *c, Connection* con) : HttpBasedTransport(c, con)
{
    _webSocket = new QWebSocket();
    _started = false;

    connect(_webSocket, SIGNAL(connected()), this, SLOT(onDisconnected()));
    connect(_webSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

}

void WebSocketTransport::start(QString)
{
    QString conOrRecon = "connect";
    if(_started)
        conOrRecon = "reconnect";
    QString connectUrl = _connection->getUrl() + "/" +conOrRecon;
    connectUrl += TransportHelper::getReceiveQueryString(_connection, _connection->onSending(), getTransportType());
    QUrl url = QUrl(connectUrl);

    if(url.scheme() == "https")
    {
        url.setScheme("wss");
    }
    else
    {
        url.setScheme("ws");
    }

    _webSocket->open(url);


}

void WebSocketTransport::send(QString data)
{
    Q_UNUSED(data);
}

void WebSocketTransport::stop()
{

}

void WebSocketTransport::abort()
{

}

const QString &WebSocketTransport::getTransportType()
{
    static QString type = "webSockets";
    return type;
}

void WebSocketTransport::onConnected()
{
    _connection->emitLogMessage("websocket connected", Connection::Debug);
}

void WebSocketTransport::onDisconnected()
{
    _connection->emitLogMessage("websocket disconnected", Connection::Debug);
}

void WebSocketTransport::onError(QAbstractSocket::SocketError)
{
    _connection->emitLogMessage(_webSocket->errorString(), Connection::Warning);
}
