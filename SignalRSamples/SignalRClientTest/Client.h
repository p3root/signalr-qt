#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTimer>

#include <Connection.h>
#include <Hubs/HubConnection.h>
#include <Transports/HttpClient.h>
#include <Transports/LongPollingTransport.h>
#include <Helper/Helper.h>
#include <Transports/ServerSentEventsTransport.h>

class Client : public QObject
{
    Q_OBJECT
public:
    Client(QCoreApplication &app);
    ~Client();

    void start();

private Q_SLOTS:
    void timerTick();
    void stop();
    void onHubMessageReceived(QVariant);
    void onError(SignalException);
    void onStateChanged(Connection::State oldState, Connection::State newState);

private:
    QTimer _timer;
    HubConnection* _connection;
    HttpClient* _client;
    ClientTransport* _transport;
};

#endif // CLIENT_H
