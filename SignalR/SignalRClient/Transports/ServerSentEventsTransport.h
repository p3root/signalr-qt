#ifndef SERVERSENTEVENTSTRANSPORT_H
#define SERVERSENTEVENTSTRANSPORT_H

#include "HttpBasedTransport.h"
#include "HttpEventStream.h"

class ServerSentEventsTransport : public QThread, public HttpBasedTransport
{
    Q_OBJECT
public:
    ServerSentEventsTransport(HttpClient* client);
    ~ServerSentEventsTransport(void);

    void start(Connection* connection, START_CALLBACK startCallback, QString data, void* state = 0);
    void abort(Connection* connection);
    void stop(Connection*);

    void run();

    const QString& getTransportType();

    struct HttpRequestInfo
    {
        START_CALLBACK callback;
        void* callbackState;
        ServerSentEventsTransport* transport;
        Connection* connection;
        QString data;
    };

    struct ReadInfo
    {
        Connection* connection;
        HttpResponse* httpResponse;
        ServerSentEventsTransport* transport;
        HttpRequestInfo* requestInfo;
    };

    void readLoop(HttpResponse& httpResponse, Connection* connection, HttpRequestInfo* reqestInfo);

private:
    static void onStartHttpResponse(HttpResponse& httpResponse, SignalException* error, void* state);
    static void onReadLine(QString data, SignalException* error, void* state);

    HttpEventStream *_eventStream;
    Connection * _connection;
    START_CALLBACK _startCallback;
    void* _state;
};

#endif
