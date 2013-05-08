#ifndef LONGPOLLINGTRANSPORT_H
#define LONGPOLLINGTRANSPORT_H

#include "HttpBasedTransport.h"
#include "Connection.h"
#include "SignalException.h"
#include <QThread>

class LongPollingTransport : public HttpBasedTransport
{
    Q_OBJECT
public:
    LongPollingTransport(HttpClient* httpClient, Connection* con);
    ~LongPollingTransport(void);

    void start(QString connection);
    void abort();
    void stop();

    const QString& getTransportType();

private Q_SLOTS:
    void onPollHttpResponse(const QString& httpResponse, SignalException *error);

private:
    bool _started;
    QString _url;
};

#endif
