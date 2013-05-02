#ifndef CONNECTIONLISTENER_H
#define CONNECTIONLISTENER_H

#include <QObject>
#include <QTcpSocket>

class ConnectionListener : public QObject
{
    Q_OBJECT

public:
    ConnectionListener(QTcpSocket* socket);

public Q_SLOTS:
    void onReadyRead();

private:
    QTcpSocket* _socket;
};

#endif // CONNECTIONLISTENER_H
