#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>
#include <SignalException.h>
#include "Transports/HttpClient.h"
#include "Transports/NegotiateResponse.h"
#include "ClientTransport.h"
#include <QDateTime>
#include <QObject>

class ConnectionHandler;
class ClientTransport;

class Connection
{
public:
    enum State
    {
        Connecting,
        Connected,
        Reconnecting,
        Disconnecting,
        Disconnected
    };

    Connection(QString url, ConnectionHandler* handler);
    virtual ~Connection(void);

    void start(bool autoReconnect = false);
    void start(ClientTransport* tranport, bool autoReconnect = false);
    void start(HttpClient* client, bool autoReconnect = false);
    virtual void stop();
    void send(QString data);
    
    State getState();
    QString getConnectionId();
    QString getConnectionToken();
    QString getGroupsToken();
    ClientTransport* getTransport();
    QString getUrl();
    QString getMessageId();
    int getPort();
    quint64 getCount();
    bool getAutoReconnect();

    bool changeState(State oldState, State newState);
    bool ensureReconnecting();
    void onError(SignalException exp);
    virtual void onReceived(QVariant data);


    void setConnectionState(NegotiateResponse negotiateResponse);
    virtual QString onSending();
    void setHeartbeat() { _hearbeat = QDateTime::currentDateTime(); }
    const QDateTime getLastHeartbeat() { return _hearbeat; }

protected:
     State _state;

private:
    QString _host;
    QString _connectionId;
    QString _connectionToken;
    QString _groupsToken;
    QString _messageId;
    ClientTransport* _transport;
    ConnectionHandler* _handler;
    quint64 _count;
    HttpClient *_httpClient;
    QDateTime _hearbeat;
    bool _autoReconnect;

    static void onTransportStartCompleted(SignalException* error, void* state);
    static void onNegotiateCompleted(NegotiateResponse* negotiateResponse, SignalException* error, void* state);
};

#endif
