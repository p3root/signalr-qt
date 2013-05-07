#ifndef CONNECTIONLISTENER_H
#define CONNECTIONLISTENER_H

#include <QObject>
#include <QTcpSocket>

#define HTTP_END_REQUEST "\r\n\r\n"

class ConnectionListener : public QObject
{
    Q_OBJECT

public:
    ConnectionListener(QTcpSocket* socket);

public Q_SLOTS:
    void onReadyRead();

Q_SIGNALS:
    void onDelete(ConnectionListener* self);

private:
    QTcpSocket* _socket;

private:
    QString getHttpRequest(QString);
};

#endif // CONNECTIONLISTENER_H
