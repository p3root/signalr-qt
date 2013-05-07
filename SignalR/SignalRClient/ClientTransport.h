#ifndef CLIENTTRANSPORT_H
#define CLIENTTRANSPORT_H

class Connection;

#include <QString>
#include "SignalException.h"
#include "Transports/NegotiateResponse.h"
#include <QObject>

class ClientTransport : public QObject
{
    Q_OBJECT
public:
    ClientTransport(void);
    virtual ~ClientTransport(void);

    virtual void negotiate() = 0;
    virtual void start(Connection* connection, QString data) = 0;
    virtual void send(Connection* connection, QString data) = 0;
    virtual void stop(Connection* connection) = 0;
    virtual void abort(Connection* connection) = 0;

Q_SIGNALS:
    void transportStarted(SignalException* ex);

private Q_SLOTS:
    virtual void negotiateCompleted(QString data, SignalException* ex) = 0;
};

#endif
