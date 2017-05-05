#include "WebSocketTransport.h"
#include "Helper/Helper.h"
#include "SignalException.h"
#include "Connection_p.h"

namespace P3 { namespace SignalR { namespace Client {

WebSocketTransport::WebSocketTransport() :
    HttpBasedTransport(),
    _webSocket(0)
{
    _started = false;
}

void WebSocketTransport::start(QString)
{
     _connection->updateLastRetryTime();

    if(_webSocket)
    {
        _webSocket->deleteLater();
        _webSocket = 0;
    }
    if(_webSocket == 0)
    {
        _webSocket = new QWebSocket();
        _webSocket->setAdditonalQueryString(_connection->getAdditionalQueryString());
        _webSocket->setAddtionalHeaders(_connection->getAdditionalHttpHeaders());
#ifndef QT_NO_NETWORKPROXY
        _webSocket->setProxy(_connection->getProxySettings());
#endif
#ifndef QT_NO_SSL
        _webSocket->setSslConfiguration(_connection->getSslConfiguration());
#endif

        QString conOrRecon = "connect";
        if(_started)
            conOrRecon = "reconnect";
        QString connectUrl = _connection->getWebSocketsUrl() + "/" +conOrRecon;
        connectUrl += TransportHelper::getReceiveQueryString(_connection, getTransportType());

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

        connect(_webSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onIgnoreSsl(QList<QSslError>)));

        _webSocket->open(url);
    }

}

void WebSocketTransport::send(QString data)
{
    if(_webSocket)
    {
        qint64 bytesWritten = _webSocket->write(data);
        if(bytesWritten != data.size())
        {
            _connection->emitLogMessage("Written bytes does not equals given bytes", SignalR::Warning);
        }
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

void WebSocketTransport::retry()
{
    abort();
    start("");
}

const QString &WebSocketTransport::getTransportType()
{
    static QString type = "webSockets";
    return type;
}

void WebSocketTransport::lostConnection(ConnectionPrivate *con)
{
    HttpBasedTransport::lostConnection(con);
    retry();
}

void WebSocketTransport::onConnected()
{
    QSharedPointer<SignalException> e;

    Q_EMIT transportStarted(e);
    _started = true;
}

void WebSocketTransport::onDisconnected()
{
    if(!_webSocket)
    {
        return;
    }
    QAbstractSocket::SocketError er = _webSocket->error();

    QSharedPointer<SignalException> error;

    switch(er)
    {
    case QAbstractSocket::RemoteHostClosedError:
        error = QSharedPointer<SignalException>(new SignalException(_webSocket->errorString(), SignalException::RemoteHostClosedConnection));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        error = QSharedPointer<SignalException>(new SignalException(_webSocket->errorString(), SignalException::ConnectionRefusedError));
        break;
    case QAbstractSocket::NetworkError:
        error = QSharedPointer<SignalException>(new SignalException(_webSocket->errorString(), SignalException::UnknownNetworkError));
        break;
    case QAbstractSocket::SocketAccessError:
    case QAbstractSocket::SocketResourceError:
    case QAbstractSocket::SocketTimeoutError:
    case QAbstractSocket::DatagramTooLargeError:
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
    case QAbstractSocket::OperationError:
    case QAbstractSocket::SslInternalError:
    case QAbstractSocket::SslInvalidUserDataError:
    case QAbstractSocket::TemporaryError:
#endif

        error = QSharedPointer<SignalException>(new SignalException(_webSocket->errorString(), SignalException::UnkownError));
        break;
    }

    if(_webSocket->state() == QAbstractSocket::ConnectedState)
        _webSocket->close();

    _connection->onError(error);

    if(_connection->ensureReconnecting())
    {
        _connection->emitLogMessage("WS: lost connection, try to reconnect in " + QString::number(_connection->getReconnectWaitTime()) + "ms", SignalR::Debug);

        connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
        _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime());
        _retryTimerTimeout.start();

    }
    else if(_connection->getAutoReconnect())
    {
        _connection->emitLogMessage("WebSocket: lost connection, try to reconnect in " + QString::number(_connection->getReconnectWaitTime()) + "ms", SignalR::Debug);

        connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
        _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime());
        _retryTimerTimeout.start();

        return;
    }
}


void WebSocketTransport::reconnectTimerTick()
{
    _retryTimerTimeout.stop();
    disconnect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
    _connection->changeState(SignalR::Connected, SignalR::Reconnecting);


    start("");
}

#ifndef QT_NO_SSL
void WebSocketTransport::onIgnoreSsl(QList<QSslError> errors)
{
    if(!_connection->ignoreSslErrors())
        return;

    _webSocket->ignoreSslErrors(errors);

    foreach(QSslError er, errors)
    {
        _connection->emitLogMessage(er.errorString(), SignalR::Error);
    }
}
#endif

void WebSocketTransport::onError(QAbstractSocket::SocketError)
{
    _connection->emitLogMessage(_webSocket->errorString(), SignalR::Warning);
}

void WebSocketTransport::onTextMessageReceived(QString str)
{
    _connection->emitLogMessage("WebSocket: Message received", SignalR::Debug);

    bool timedOut = false, disconnected = false;
    quint64 messageId = 0;
    _connection->updateLastKeepAlive();

    if(_connection->getState() != SignalR::Connected)
        _connection->changeState(_connection->getState(), SignalR::Connected);

    QSharedPointer<SignalException> e = TransportHelper::processMessages(_connection, str, &timedOut, &disconnected, &messageId);

    if(!e.isNull())
    {
        _connection->onError(e);
    }

    Q_EMIT onMessageSentCompleted(e, messageId);
}

void WebSocketTransport::onPong(quint64, QByteArray)
{
    _connection->emitLogMessage("on pong", SignalR::Debug);
}

}}}
