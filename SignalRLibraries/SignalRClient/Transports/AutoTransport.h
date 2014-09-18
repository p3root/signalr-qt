#ifndef AUTOTRANSPORT_H
#define AUTOTRANSPORT_H

#include "HttpBasedTransport.h"
#include "SignalR.h"

namespace P3 { namespace SignalR { namespace Client {

class SIGNALR_EXPORT AutoTransport : public HttpBasedTransport
{
    Q_OBJECT

public:
    AutoTransport();
    ~AutoTransport();

    void negotiate() OVERRIDE_M;
    void onNegotiatenCompleted(const NegotiateResponse& res) OVERRIDE_M;
    void start(QString connection) OVERRIDE_M;
    bool abort(int timeoutMs = 0) OVERRIDE_M;
    void send(QString data) OVERRIDE_M;

    void lostConnection(ConnectionPrivate *con) OVERRIDE_M;

    void retry() OVERRIDE_M;

    bool supportsKeepAlive() OVERRIDE_M;

    const QString &getTransportType() OVERRIDE_M;

private Q_SLOTS:
    void onTransportStated(QSharedPointer<SignalException> e);
    void onMessageSent(QSharedPointer<SignalException> ex, quint64 messageId);

private:
    QList<ClientTransport*> _transports;
    int _index;
    ClientTransport *_transport;

    QQueue<QString> _messages;

};

}}}

#endif // AUTOTRANSPORT_H
