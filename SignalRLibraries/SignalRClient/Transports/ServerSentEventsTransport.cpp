#include "ServerSentEventsTransport.h"
#include "Helper/Helper.h"

ServerSentEventsTransport::ServerSentEventsTransport(HttpClient* httpClient, Connection* con) : HttpBasedTransport(httpClient, con), _eventStream(0)
{

}

ServerSentEventsTransport::~ServerSentEventsTransport(void)
{

}

void ServerSentEventsTransport::start(QString)
{
    _url = _connection->getUrl() + "/connect";
    _url += TransportHelper::getReceiveQueryString(_connection, _connection->onSending(), getTransportType());

    QUrl qurl = QUrl(_url);

    if(_eventStream)
        delete _eventStream;

    _eventStream = new HttpEventStream(qurl);

    connect(_eventStream, SIGNAL(packetReady(QString,SignalException*)), this, SLOT(packetReceived(QString,SignalException*)));
    connect(_eventStream, SIGNAL(connected(SignalException*)), this, SLOT(connected(SignalException*)));

    _eventStream->start();

}

void ServerSentEventsTransport::abort()
{
    _eventStream->close();
    _eventStream->deleteLater();
}

void ServerSentEventsTransport::stop()
{
    _eventStream->close();
}

const QString &ServerSentEventsTransport::getTransportType()
{
    static QString type = "serverSentEvents";
    return type;
}

void ServerSentEventsTransport::packetReceived(QString data, SignalException *error)
{
    bool timedOut = false, disconnected = false;

    if(data == "data: initialized")
    {
        if(_started)
        {
            Q_EMIT transportStarted(0);
        }
        else
        {
            // Reconnected
            _connection->changeState(Connection::Reconnecting, Connection::Connected);
        }
    }
    else if(error != 0)
    {
        if(error->getType() == SignalException::EventStreamSocketLost)
        {
            reconnect();
        }
        else if(_connection->ensureReconnecting())
        {
            QLOG_DEBUG() << "ServerSentEventsTransport: Lost connection...try to reconnect";

            //wait to seconds before reconnecting
            Helper::wait(2);
            reconnect();
        }
        else if(_connection->getAutoReconnect())
        {
            QLOG_DEBUG() << "ServerSentEventsTransport: (autoconnect=true)  Lost connection...try to reconnect";


            //wait to seconds before reconnecting
            Helper::wait(2);
            reconnect();
        }
        else
        {
            _connection->onError(*error);
        }

        delete error;
    }
    else
    {
        data = data.remove(0, data.indexOf("data: ")+5);
        _connection->setHeartbeat();
        TransportHelper::processMessages(_connection, data, &timedOut, &disconnected);
    }

    if(disconnected)
    {
        _connection->stop();
    }
}

void ServerSentEventsTransport::connected(SignalException *error)
{
    Q_EMIT transportStarted(error);
}

void ServerSentEventsTransport::reconnect()
{
    _eventStream->close();
    _eventStream->start();
}


/*
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
        if(requestInfo->connection->getAutoReconnect())
        {
            QLOG_DEBUG() << "ServerSentEventsTransport: Could not establish connection...try it again";

            Helper::wait(2);
            ((ServerSentEventsTransport*)requestInfo->transport)->run();
            return;
        }
        else
        {
            QLOG_DEBUG() << "ServerSentEventsTransport: (autoconnect=true) Could not establish connection...will not try it again";
            requestInfo->callback(error, requestInfo->callbackState);
        }
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
    bool timedOut = false, disconnected = false;

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
            QLOG_DEBUG() << "ServerSentEventsTransport: Lost connection...try to reconnect";

            //wait to seconds before reconnecting
            Helper::wait(2);
            ((ServerSentEventsTransport*)readInfo->requestInfo->transport)->run();

            return;
        }
        else if(readInfo->connection->getAutoReconnect())
        {
            QLOG_DEBUG() << "ServerSentEventsTransport: (autoconnect=true)  Lost connection...try to reconnect";


            //wait to seconds before reconnecting
            Helper::wait(2);
            ((ServerSentEventsTransport*)readInfo->requestInfo->transport)->run();
            return;
        }
        else
        {
            readInfo->connection->onError(*error);
        }
    }
    else
    {
        data = data.remove(0, data.indexOf("data: ")+5);
        readInfo->connection->setHeartbeat();
        TransportHelper::processMessages(readInfo->connection, data, &timedOut, &disconnected);
    }

    if(disconnected)
    {
        readInfo->connection->stop();
    }
    else
    {
        readInfo->transport->readLoop(*readInfo->httpResponse, readInfo->connection, readInfo->requestInfo);
    }

    delete readInfo;
}*/
