#ifndef WEBSOCKETTRANSPORT_H
#define WEBSOCKETTRANSPORT_H

#include "HttpBasedTransport.h"
#include <qwebsocket.h>
#include "Connection.h"
#include "Helper/TransportHelper.h"
#include <QTimer>


namespace P3 { namespace SignalR { namespace Client {

class SIGNALR_EXPORT WebSocketTransport : public HttpBasedTransport
{
    Q_OBJECT
public:
    WebSocketTransport();

    void start(QString data);
    void send(QString data);
    bool abort(int timeoutMs=0);
    void retry();

    virtual const QString &getTransportType();

    void lostConnection(ConnectionPrivate *con);

    bool supportsKeepAlive() { return true; }

private Q_SLOTS:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError);

    void onTextMessageReceived(QString);
    void onPong(quint64, QByteArray);

    void reconnectTimerTick();

#ifndef QT_NO_SSL
    void onIgnoreSsl(QList<QSslError>);
#endif

private:
    QWebSocket *_webSocket;
    bool _started;
    QTimer _retryTimerTimeout;
};


}}}

#endif // WEBSOCKETTRANSPORT_H
