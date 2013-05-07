#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTimer>

#include <Connection.h>
#include <Hubs/HubConnection.h>
#include <MyConnectionHandler.h>
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

private:
    QTimer _timer;
    MyConnectionHandler* _handler;
    HubConnection* _connection;
    HttpClient* _client;
    ClientTransport* _transport;
};

#endif // CLIENT_H
