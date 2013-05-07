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

    void start(Connection* connection, QString data);
    void abort(Connection *connection);
    void stop(Connection *connection);

    const QString& getTransportType();

private Q_SLOTS:
    void onPollHttpResponse(const QString& httpResponse, SignalException *error);

private:
    bool _started;
    QString _url;
};

#endif
