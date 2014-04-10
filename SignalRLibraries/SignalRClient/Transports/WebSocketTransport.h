#ifndef WEBSOCKETTRANSPORT_H
#define WEBSOCKETTRANSPORT_H

#include "HttpBasedTransport.h"
#include <qwebsocket.h>
#include "Connection.h"
#include "Helper/TransportHelper.h"
#include <QTimer>


namespace P3 { namespace SignalR { namespace Client {

class WebSocketTransport : public HttpBasedTransport
{
    Q_OBJECT
public:
    WebSocketTransport(HttpClient *c, Connection* con);

    void start(QString data);
    void send(QString data);
    bool abort(int timeoutMs=0);
    void retry();

    virtual const QString &getTransportType();

private Q_SLOTS:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError);

    void onTextMessageReceived(QString);
    void onPong(quint64, QByteArray);

    void keepAliveTimerTimeout();

    void reconnectTimerTick();

#ifndef QT_NO_SSL
    void onIgnoreSsl(QList<QSslError>);
#endif

private:
    QWebSocket *_webSocket;
    QTimer _keepAliveTimer;
    bool _started;
    QTimer _retryTimerTimeout;
};


}}}

#endif // WEBSOCKETTRANSPORT_H
