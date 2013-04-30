#include "ServerSentEventsTransport.h"
#include "Helper/Helper.h"

ServerSentEventsTransport::ServerSentEventsTransport(HttpClient* httpClient) : HttpBasedTransport(httpClient)
{

}

ServerSentEventsTransport::~ServerSentEventsTransport(void)
{

}

void ServerSentEventsTransport::start(Connection* connection, START_CALLBACK startCallback, QString, void* state)
{
    _connection = connection;
    _startCallback = startCallback;
    _state = state;

    QThread::start();
}

void ServerSentEventsTransport::abort(Connection*)
{
    _eventStream->abort();
}

void ServerSentEventsTransport::stop(Connection*)
{
    _eventStream->abort();

}

void ServerSentEventsTransport::run()
{
    QString url = _connection->getUrl();

    if(_startCallback != NULL)
    {
        url += "/connect";
    }

    url += TransportHelper::getReceiveQueryString(_connection, _connection->onSending(), getTransportType());

    HttpRequestInfo* requestInfo = new HttpRequestInfo();
    requestInfo->callbackState = _state;
    requestInfo->transport = this;
    requestInfo->callback = _startCallback;
    requestInfo->connection = _connection;
    requestInfo->data = _connection->onSending();

    QUrl qurl = QUrl(url);
    _eventStream = new HttpEventStream();
    _eventStream->get(qurl, &ServerSentEventsTransport::onStartHttpResponse, requestInfo);
}

const QString &ServerSentEventsTransport::getTransportType()
{
    static QString type = "serverSentEvents";
    return type;
}

void ServerSentEventsTransport::onStartHttpResponse(HttpResponse& httpResponse, SignalException* error, void* state)
{
    HttpRequestInfo* requestInfo = (HttpRequestInfo*)state;

    if(!error)
    {
        requestInfo->callback(0, requestInfo->callbackState);
        requestInfo->transport->readLoop(httpResponse, requestInfo->connection, requestInfo);
    }
    else
    {
        requestInfo->callback(error, requestInfo->callbackState);
        delete requestInfo;
    }
}

void ServerSentEventsTransport::readLoop(HttpResponse& httpResponse, Connection* connection, HttpRequestInfo* requestInfo)
{
    ReadInfo* readInfo = new ReadInfo();

    readInfo->httpResponse = &httpResponse;
    readInfo->connection = connection;
    readInfo->requestInfo = requestInfo;

    httpResponse.readLine(&ServerSentEventsTransport::onReadLine, readInfo);
}

void ServerSentEventsTransport::onReadLine(QString data, SignalException* error, void* state)
{
    ReadInfo* readInfo = (ReadInfo*)state;
    bool timedOut, disconnected;

    if(data == "data: initialized")
    {
        if(!readInfo->requestInfo->callback)
        {
            readInfo->requestInfo->callback(NULL, readInfo->requestInfo->callbackState);
            readInfo->requestInfo->callback = NULL;
        }
        else
        {
            // Reconnected
            readInfo->connection->changeState(Connection::Reconnecting, Connection::Connected);
        }
    }
    else if(error != 0)
    {
        if(readInfo->connection->ensureReconnecting())
        {
            sleep(1000);
            readInfo->transport->start(readInfo->connection, NULL, readInfo->requestInfo->data);
            return;
        }
        else
        {
            readInfo->connection->onError(*error);
        }
    }
    else
    {
        data = data.remove(0, data.indexOf("data: ")+6);
        data = data.simplified();
        TransportHelper::processMessages(readInfo->connection, data, &timedOut, &disconnected);
    }

    if(disconnected)
    {
        readInfo->connection->stop();
    }
    else
    {
        readInfo->transport->readLoop(*readInfo->httpResponse, readInfo->connection, NULL);
    }

    data = "";


    delete readInfo;
}
