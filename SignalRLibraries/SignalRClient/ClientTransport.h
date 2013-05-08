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
    ClientTransport(Connection* con);
    virtual ~ClientTransport(void);

    virtual void negotiate() = 0;
    virtual void start(QString data) = 0;
    virtual void send(QString data) = 0;
    virtual void stop() = 0;
    virtual void abort() = 0;

Q_SIGNALS:
    void transportStarted(SignalException* ex);

private Q_SLOTS:
    virtual void negotiateCompleted(QString data, SignalException* ex) = 0;

protected:
    Connection* _connection;
};

#endif
