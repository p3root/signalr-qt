#ifndef ICONNECTIONHANDLER_H
#define ICONNECTIONHANDLER_H

#include <QString>
#include <SignalException.h>
#include <QsLog.h>
#include "Connection.h"


class ConnectionHandler
{
public:
    ConnectionHandler(void);
    virtual ~ConnectionHandler(void);

    virtual void onStateChanged(Connection::State old_state, Connection::State new_state) = 0;
    virtual void onError(SignalException error) = 0;
    virtual void onReceived(QString data) = 0;
};

#endif
