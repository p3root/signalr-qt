#include "LongPollingTransport.h"
#include "Helper/Helper.h"
#include <QsLog.h>

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
    //mHttpClient->moveToThread(this);
    QThread::start();
}

void LongPollingTransport::abort(Connection *)
{
    mHttpClient->abort();
}

void LongPollingTransport::stop(Connection *)
{
    mHttpClient->abort();
}

void LongPollingTransport::run()
{
    QString url = _connection->getUrl();

    if(_startCallback != NULL)
    {
        url += "/connect";
    }

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
    info->callback = 0;
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
            if(error->getType() == SignalException::ConnectionRefusedError)
            {
                pollInfo->connection->changeState(Connection::Reconnecting, Connection::Connected);
            }

            if(pollInfo->connection->ensureReconnecting())
            {
                QLOG_DEBUG() << "LongPollingTranpsort: lost connection...try to reconnect";
                Helper::wait(2);
                pollInfo->transport->run();
            }
            else if(pollInfo->connection->getAutoReconnect())
            {
                QLOG_DEBUG() << "LongPollingTranpsort: (autoconnect=true) lost connection...try to reconnect";
                Helper::wait(2);
                pollInfo->transport->run();
            }
            else
            {
                pollInfo->connection->onError(*error);
            }
        }

        delete error;
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
