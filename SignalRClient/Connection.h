#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>
#include <SignalException.h>
#include "Transports/HttpClient.h"
#include "Transports/NegotiateResponse.h"
#include "ClientTransport.h"

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
    ~Connection(void);

    void start();
    void start(ClientTransport* tranport);
    void start(HttpClient* client);
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

    // Transport API
    bool changeState(State oldState, State newState);
    bool ensureReconnecting();
    void onError(SignalException exp);
    virtual void onReceived(QString data);

    void setConnectionState(NegotiateResponse negotiateResponse);
    virtual QString onSending();

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

    static void onTransportStartCompleted(SignalException* error, void* state);
    static void onNegotiateCompleted(NegotiateResponse* negotiateResponse, SignalException* error, void* state);
};

#endif
