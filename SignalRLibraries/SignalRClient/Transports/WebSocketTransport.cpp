#include "WebSocketTransport.h"
#include "Helper/Helper.h"
#include "SignalException.h"

namespace P3 { namespace SignalR { namespace Client {

WebSocketTransport::WebSocketTransport(HttpClient *c, Connection* con) : HttpBasedTransport(c, con), _webSocket(0)
{
    connect(&_keepAliveTimer, SIGNAL(timeout()), this, SLOT(keepAliveTimerTimeout()));
    _keepAliveTimer.setSingleShot(true);
    _started = false;
}

void WebSocketTransport::start(QString)
{
    if(_webSocket == 0)
    {
        _webSocket = new QWebSocket();
        _webSocket->setAdditonalQueryString(_connection->getAdditionalQueryString());
        _webSocket->setAddtionalHeaders(_connection->getAdditionalHttpHeaders());

        QString conOrRecon = "connect";
        if(_started)
            conOrRecon = "reconnect";
        QString connectUrl = _connection->getWebSocketsUrl() + "/" +conOrRecon;
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
        connect(_webSocket, SIGNAL(connected()), this, SLOT(onConnected()));
        connect(_webSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
        connect(_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

        connect(_webSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
        connect(_webSocket, SIGNAL(pong(quint64,QByteArray)), this, SLOT(onPong(quint64,QByteArray)));

        _keepAliveTimer.stop();
        _keepAliveTimer.setInterval(_connection->getKeepAliveData().getTimeout()*1000);
        _keepAliveTimer.start();

        _webSocket->open(url);
    }

}

void WebSocketTransport::send(QString data)
{
    if(_webSocket)
    {
        //QString writeData = QString("{%1}").arg(data);
        qint64 bytesWritten = _webSocket->write(data);
        Q_UNUSED(bytesWritten);
    }
}

bool WebSocketTransport::abort(int timeoutMs)
{
    Q_UNUSED(timeoutMs);
    if(_webSocket)
    {
        _webSocket->close();
        _webSocket->deleteLater();
        _webSocket = 0;
    }
    return true;
}

const QString &WebSocketTransport::getTransportType()
{
    static QString type = "webSockets";
    return type;
}

void WebSocketTransport::onConnected()
{
    _connection->emitLogMessage("websocket connected", Connection::Debug);
    Q_EMIT transportStarted(0);
}

void WebSocketTransport::onDisconnected()
{
    _keepAliveTimer.stop();
    if(!_webSocket)
    {
        return;
    }
    QAbstractSocket::SocketError er = _webSocket->error();

    SignalException *error = 0;

    switch(er)
    {
    case QAbstractSocket::RemoteHostClosedError:
    case QAbstractSocket::ConnectionRefusedError:
    case QAbstractSocket::SocketAccessError:
    case QAbstractSocket::SocketResourceError:
    case QAbstractSocket::SocketTimeoutError:
    case QAbstractSocket::DatagramTooLargeError:
    case QAbstractSocket::NetworkError:
    case QAbstractSocket::AddressInUseError:
    case QAbstractSocket::SocketAddressNotAvailableError:
    case QAbstractSocket::UnsupportedSocketOperationError:
    case QAbstractSocket::UnfinishedSocketOperationError:
    case QAbstractSocket::ProxyAuthenticationRequiredError:
    case QAbstractSocket::SslHandshakeFailedError:
    case QAbstractSocket::ProxyConnectionRefusedError:
    case QAbstractSocket::ProxyConnectionClosedError:
    case QAbstractSocket::ProxyConnectionTimeoutError:
    case QAbstractSocket::ProxyNotFoundError:
    case QAbstractSocket::ProxyProtocolError:
    case QAbstractSocket::UnknownSocketError:
    case QAbstractSocket::HostNotFoundError:
        error = new SignalException();
        break;
    }

    if(_webSocket->state() == QAbstractSocket::ConnectedState)
        _webSocket->close();
    _webSocket->deleteLater();
    _webSocket = 0;

    _connection->onError(*error);

    if(_connection->ensureReconnecting())
    {
        if(_connection->getLogErrorsToQDebug())
            qDebug() << "ServerSentEventsTransport: Lost connection...try to reconnect";
        _connection->emitLogMessage("lost connection...try to reconnect", Connection::Debug);

        connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
        _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime() * 1000);
        _retryTimerTimeout.start();

    }
    else if(_connection->getAutoReconnect())
    {
        if(_connection->getLogErrorsToQDebug())
            qDebug() << "ServerSentEventsTransport: (autoconnect=true)  Lost connection...try to reconnect";
        _connection->emitLogMessage("lost connection...try to reconnect", Connection::Debug);

        _connection->changeState(Connection::Connected, Connection::Reconnecting);

        connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
        _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime() * 1000);
        _retryTimerTimeout.start();

        return;
    }
    else
    {
        _connection->onError(*error);
    }

    delete error;

    _started = false;
    _connection->emitLogMessage("websocket disconnected ("+QString::number(er)+")", Connection::Debug);

}


void WebSocketTransport::reconnectTimerTick()
{
    _retryTimerTimeout.stop();
    disconnect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
    start("");
}

void WebSocketTransport::onError(QAbstractSocket::SocketError)
{
    _connection->emitLogMessage(_webSocket->errorString(), Connection::Warning);
}

void WebSocketTransport::onTextMessageReceived(QString str)
{
    _keepAliveTimer.stop();
    bool timedOut = false, disconnected = false;
    _connection->updateLastKeepAlive();

    TransportHelper::processMessages(_connection, str, &timedOut, &disconnected);

    _connection->emitLogMessage("text message received (" + str +")", Connection::Debug);
    _keepAliveTimer.start();
}

void WebSocketTransport::onPong(quint64, QByteArray)
{
    _connection->emitLogMessage("on pong", Connection::Debug);
}

void WebSocketTransport::keepAliveTimerTimeout()
{
    _connection->emitLogMessage("websocket timed out", Connection::Debug);

    if(_webSocket->state() == QAbstractSocket::ConnectedState)
        _webSocket->close();
    else
    {
        _webSocket->deleteLater();
        _webSocket = 0;

        connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
        _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime() * 1000);
        _retryTimerTimeout.start();
    }
}

}}}
