#ifndef ICONNECTIONHANDLER_H
#define ICONNECTIONHANDLER_H

#include <QString>
#include <SignalException.h>
#include <QsLog.h>
#include "Connection.h"
#include <QObject>
#include <QThread>

class ConnectionHandler : public QObject
{
    Q_OBJECT
public:
    ConnectionHandler(void);
    virtual ~ConnectionHandler(void);

    void receivedData(QVariant data);

Q_SIGNALS:
    void onReceivedData(QVariant data);

public Q_SLOTS:
    virtual void onStateChanged(Connection::State old_state, Connection::State new_state) = 0;
    virtual void onError(SignalException error) = 0;
    virtual void onReceived(QVariant data) = 0;
};

#endif
