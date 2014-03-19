#ifndef AUTOTRANSPORT_H
#define AUTOTRANSPORT_H

#include "HttpBasedTransport.h"

class AutoTransport  : public HttpBasedTransport
{
    Q_OBJECT

public:
    AutoTransport(HttpClient* httpClient, Connection* con);
    ~AutoTransport();

    void onNegotiatenCompleted(const NegotiateResponse& res);
    void start(QString connection);
    bool abort(int timeoutMs = 0);
    void send(QString data);

    const QString &getTransportType();

private Q_SLOTS:
    void onTransportStated(SignalException *e);

private:
    QList<ClientTransport*> _transports;
    int _index;
    ClientTransport *_transport;

};

#endif // AUTOTRANSPORT_H
