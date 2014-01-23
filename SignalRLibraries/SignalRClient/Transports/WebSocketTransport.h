#ifndef WEBSOCKETTRANSPORT_H
#define WEBSOCKETTRANSPORT_H

#include "HttpBasedTransport.h"
#include <qwebsocket.h>
#include "Connection.h"
#include "Helper/TransportHelper.h"

class WebSocketTransport : public HttpBasedTransport
{
    Q_OBJECT
public:
    WebSocketTransport(HttpClient *c, Connection* con);

    virtual void start(QString data);
    virtual void send(QString data);
    virtual void stop();
    virtual void abort();

     virtual const QString &getTransportType();

private Q_SLOTS:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError);

    void onTextMessageReceived(QString);
    void onPong(quint64, QByteArray);

private:
    QWebSocket *_webSocket;
    bool _started;
};

#endif // WEBSOCKETTRANSPORT_H
