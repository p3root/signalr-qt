#include "HttpBasedTransport.h"


HttpBasedTransport::HttpBasedTransport(HttpClient* httpClient) : _sending(false)
{
    mHttpClient = httpClient;
}


HttpBasedTransport::~HttpBasedTransport(void)
{

}

void HttpBasedTransport::negotiate(Connection* connection, NEGOTIATE_CALLBACK negotiateCallback, void* state)
{
    TransportHelper::getNegotiationResponse(mHttpClient, connection, negotiateCallback, state);
}

void HttpBasedTransport::send(Connection* connection, QString data)
{
    QString url = connection->getUrl() +
        "/send";

    url += TransportHelper::getReceiveQueryString(connection, "", getTransportType());

    QMap<QString, QString> postData;
    postData.insert("data",data);

    if(_sending)
    {
        SendQueueItem *queueItem = new SendQueueItem();
        queueItem->connection = connection;
        queueItem->url = url;
        queueItem->postData = postData;
        _sendQueue.append(queueItem);
    }
    else
    {
        mHttpClient->post(url, postData, &HttpBasedTransport::onSendHttpResponse, this);
    }
}

void HttpBasedTransport::tryDequeueNextWorkItem()
{
    // If the queue is empty then we are free to send
    _sending = _sendQueue.size() > 0;

    if(_sending)
    {
        // Grab the next work item from the queue
        SendQueueItem* workItem = _sendQueue.front();

        // Nuke the work item
        _sendQueue.dequeue();

        mHttpClient->post(workItem->url, workItem->postData, &HttpBasedTransport::onSendHttpResponse, this);

        delete workItem;
    }
}

void HttpBasedTransport::onSendHttpResponse(const QString& httpResponse, SignalException* error, void* state)
{    
    Q_UNUSED(httpResponse);
    Q_UNUSED(error);

    HttpBasedTransport* transport = (HttpBasedTransport*)state;

    transport->tryDequeueNextWorkItem();
}

void HttpBasedTransport::stop(Connection* connection)
{
    Q_UNUSED(connection)
}


void HttpBasedTransport::abort(Connection* connection)
{
    Q_UNUSED(connection)
}

