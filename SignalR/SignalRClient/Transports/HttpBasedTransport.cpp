#include "HttpBasedTransport.h"


HttpBasedTransport::HttpBasedTransport(HttpClient* httpClient, Connection *con) : _sending(false), _connection(0)
{
    mHttpClient = httpClient;
    _connection = con;
}


HttpBasedTransport::~HttpBasedTransport(void)
{

}

void HttpBasedTransport::negotiateCompleted(QString data, SignalException *ex)
{
    if(!ex)
    {
        disconnect(mHttpClient, SIGNAL(getRequestCompleted(QString,SignalException*)), this, SLOT(negotiateCompleted(QString,SignalException*)));
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

    connect(mHttpClient, SIGNAL(getRequestCompleted(QString,SignalException*)), this, SLOT(negotiateCompleted(QString,SignalException*)));
    mHttpClient->get(url);
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
        //TODO: POST
        //mHttpClient->post(url, postData, &HttpBasedTransport::onSendHttpResponse, this);
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

        //TODO: POST
        //mHttpClient->post(workItem->url, workItem->postData, &HttpBasedTransport::onSendHttpResponse, this);

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

void HttpBasedTransport::stop(Connection*)
{
    //emit aboutToClose();
}


void HttpBasedTransport::abort(Connection*)
{
    //emit aboutToAbort();
}

