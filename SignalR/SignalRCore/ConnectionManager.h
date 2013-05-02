#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QObject>
#include <SignalRServer.h>
#include <QTcpServer>
#include <QThread>
#include <ConnectionListener.h>

class ConnectionManager : public QThread
{
    Q_OBJECT
public:
    explicit ConnectionManager(SignalRServer &server);

    void startListener();
    void stop();

    void run();

private Q_SLOTS:
    void acceptConnection();
    
private:
    SignalRServer& _server;
    QTcpServer* _connection;
    bool _isRunning;
    QList<ConnectionListener*> _listeners;
    
};

#endif // CONNECTIONMANAGER_H
