#include "HttpBasedTransport.h"


HttpBasedTransport::HttpBasedTransport(HttpClient* httpClient, Connection *con) : _sending(false), _connection(0)
{
    _httpClient = httpClient;
    _connection = con;
}


HttpBasedTransport::~HttpBasedTransport(void)
{

}

void HttpBasedTransport::negotiateCompleted(QString data, SignalException *ex)
{
    if(!ex)
    {
        disconnect(_httpClient, SIGNAL(getRequestCompleted(QString,SignalException*)), this, SLOT(negotiateCompleted(QString,SignalException*)));
        const NegotiateResponse* res = TransportHelper::parseNegotiateHttpResponse(data);

        if(res)
        {
            _connection->negotiateCompleted(res, ex);
            delete res;
        }
    }
    else
    {
         _connection->negotiateCompleted(0, ex);
    }

}

void HttpBasedTransport::negotiate()
{
    QString url = _connection->getUrl() + "/negotiate";

    connect(_httpClient, SIGNAL(getRequestCompleted(QString,SignalException*)), this, SLOT(negotiateCompleted(QString,SignalException*)));
    _httpClient->get(url);
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
        connect(_httpClient, SIGNAL(postRequestCompleted(QString,SignalException*)), this, SLOT(onSendHttpResponse(QString,SignalException*)));
        _httpClient->post(url, postData);
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

        connect(_httpClient, SIGNAL(postRequestCompleted(QString,SignalException*)), this, SLOT(onSendHttpResponse(QString,SignalException*)));
        _httpClient->post(workItem->url, workItem->postData);

        delete workItem;
    }
}

void HttpBasedTransport::onSendHttpResponse(const QString& httpResponse, SignalException* error)
{    
    Q_UNUSED(httpResponse);
    Q_UNUSED(error);

    if(!error)
    {
        //TODO error handling
    }
    disconnect(_httpClient, SIGNAL(postRequestCompleted(QString,SignalException*)), this, SLOT(onSendHttpResponse(QString,SignalException*)));
    tryDequeueNextWorkItem();
}

void HttpBasedTransport::stop(Connection*)
{
    //emit aboutToClose();
}


void HttpBasedTransport::abort(Connection*)
{
    //emit aboutToAbort();
}

