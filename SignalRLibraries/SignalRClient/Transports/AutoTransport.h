#ifndef AUTOTRANSPORT_H
#define AUTOTRANSPORT_H

#include "HttpBasedTransport.h"


namespace P3 { namespace SignalR { namespace Client {

class SIGNALR_EXPORT AutoTransport : public HttpBasedTransport
{
    Q_OBJECT

public:
    AutoTransport();
    ~AutoTransport();

    void negotiate();
    void onNegotiatenCompleted(const NegotiateResponse& res);
    void start(QString connection);
    bool abort(int timeoutMs = 0);
    void send(QString data);

    void lostConnection(ConnectionPrivate *con);

    void retry();

    bool supportsKeepAlive();

    const QString &getTransportType();

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
