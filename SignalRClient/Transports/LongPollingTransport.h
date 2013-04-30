#ifndef LONGPOLLINGTRANSPORT_H
#define LONGPOLLINGTRANSPORT_H

#include "HttpBasedTransport.h"
#include "Connection.h"
#include "SignalException.h"
#include <QThread>

class LongPollingTransport : public QThread, public HttpBasedTransport
{
    Q_OBJECT
public:
    LongPollingTransport(HttpClient* httpClient);
    ~LongPollingTransport(void);

    void start(Connection* connection, START_CALLBACK startCallback, QString data, void* state = 0);
    void abort(Connection *connection);
    void stop(Connection *connection);

    void run();

    const QString& getTransportType();

    struct PollHttpRequestInfo
    {
        START_CALLBACK callback;
        void* callbackState;
        LongPollingTransport* transport;
        Connection* connection;
        QString data;
        HttpClient* client;
    };

private:
    static void onPollHttpResponse(const QString& httpResponse, SignalException* error, void* state);

    Connection * _connection;
    START_CALLBACK _startCallback;
    void* _state;
};

#endif
