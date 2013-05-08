#ifndef SERVERSENTEVENTSTRANSPORT_H
#define SERVERSENTEVENTSTRANSPORT_H

#include "HttpBasedTransport.h"
#include "HttpEventStream.h"

class ServerSentEventsTransport : public HttpBasedTransport
{
    Q_OBJECT
public:
    ServerSentEventsTransport(HttpClient* client, Connection *con);
    ~ServerSentEventsTransport(void);

    void start(QString data);
    void abort();
    void stop();

    const QString& getTransportType();

private Q_SLOTS:
    void packetReceived(QString packet, SignalException *ex);
    void connected(SignalException* ex);


private:
    void reconnect();

private:
    HttpEventStream *_eventStream;
    void* _state;
    QString _url;
    bool _started;
};

#endif
