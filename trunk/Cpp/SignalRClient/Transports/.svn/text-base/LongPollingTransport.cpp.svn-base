#include "LongPollingTransport.h"


LongPollingTransport::LongPollingTransport(HttpClient* httpClient) :HttpBasedTransport(httpClient)
{

}


LongPollingTransport::~LongPollingTransport(void)
{
}

void LongPollingTransport::start(Connection* connection, START_CALLBACK startCallback, QString, void* state)
{    
    _connection = connection;
    _startCallback = startCallback;
    _state = state;
    mHttpClient->moveToThread(this);
    QThread::start();



    // TODO: Need to set a timer here to trigger connected after 2 seconds or so
}

void LongPollingTransport::run()
{
    QString url = _connection->getUrl();

    if(_startCallback != NULL)
    {
        url += "/connect";
    }

    // TODO: Handle reconnect

    url += TransportHelper::getReceiveQueryString(_connection, _connection->onSending(), getTransportType());

    PollHttpRequestInfo* info = new PollHttpRequestInfo();
    info->callbackState = _state;
    info->transport = this;
    info->callback = _startCallback;
    info->connection = _connection;
    info->data = url;
    info->client = mHttpClient;

    if(info->callback != NULL)
    {
        info->callback(0, info->callbackState);
    }

    mHttpClient->get(url, &LongPollingTransport::onPollHttpResponse,  info);
}

const QString &LongPollingTransport::getTransportType()
{
    static QString type  ="longPolling";
    return type;
}

void LongPollingTransport::onPollHttpResponse(const QString& httpResponse, SignalException * error, void* state)
{
    PollHttpRequestInfo* pollInfo = (PollHttpRequestInfo*)state;
    bool timedOut, disconnected;

    if(!error)
    {        
        TransportHelper::processMessages(pollInfo->connection, httpResponse, &timedOut, &disconnected);
    }
    else
    {
        if(pollInfo->callback != NULL) 
        {
            pollInfo->callback(error, pollInfo->callbackState);
        }
        else
        {
            pollInfo->connection->onError(*error);
        }
    }

    if(disconnected)
    {
        pollInfo->connection->stop();
    }
    else
    {
        pollInfo->client->get(pollInfo->data, &LongPollingTransport::onPollHttpResponse,  pollInfo);
    }

    delete pollInfo;
}
