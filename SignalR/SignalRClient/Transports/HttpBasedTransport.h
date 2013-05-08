#ifndef HTTPBASEDTRANSPORT_H
#define HTTPBASEDTRANSPORT_H

#include "ClientTransport.h"
#include "Connection.h"
#include "Transports/HttpClient.h"
#include "Helper/TransportHelper.h"
#include "SignalException.h"
#include <QQueue>

class HttpBasedTransport : public ClientTransport
{
    Q_OBJECT
protected:
    HttpClient* _httpClient;

public:
    HttpBasedTransport(HttpClient* httpClient, Connection* con);
    ~HttpBasedTransport(void);

    void negotiateCompleted(QString data, SignalException* ex);
    void negotiate();
    void send(QString data);
    virtual void stop();
    void abort();

    virtual const QString& getTransportType() = 0;

    void tryDequeueNextWorkItem();
private:    
    
    struct SendQueueItem
    {
        Connection* connection;
        QString url;
        QMap<QString, QString> postData;
    };
    
    QQueue<SendQueueItem*> _sendQueue;
    bool _sending;

private Q_SLOTS:
    void onSendHttpResponse(const QString& httpResponse, SignalException* error);
};

#endif
