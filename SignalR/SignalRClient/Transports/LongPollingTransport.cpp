#include "LongPollingTransport.h"
#include "Helper/Helper.h"
#include <QsLog.h>

LongPollingTransport::LongPollingTransport(HttpClient* httpClient, Connection *con) :HttpBasedTransport(httpClient, con)
{

}


LongPollingTransport::~LongPollingTransport(void)
{
}

void LongPollingTransport::start(Connection*, QString)
{
    connect(mHttpClient, SIGNAL(getRequestCompleted(QString,SignalException*)), this, SLOT(onPollHttpResponse(QString,SignalException*)));

    _url = _connection->getUrl() + "/connect";
    _url += TransportHelper::getReceiveQueryString(_connection, _connection->onSending(), getTransportType());

    Q_EMIT transportStarted(0);

    _started = true;
    mHttpClient->get(_url);
}

void LongPollingTransport::abort(Connection *)
{
    mHttpClient->abort();
}

void LongPollingTransport::stop(Connection *)
{
    mHttpClient->abort();
}

const QString &LongPollingTransport::getTransportType()
{
    static QString type  ="longPolling";
    return type;
}

void LongPollingTransport::onPollHttpResponse(const QString& httpResponse, SignalException *ex)
{ 
    bool timedOut = false, disconnected = false;
    SignalException *error = 0;
    if(ex)
       error = (SignalException*)ex;

    if(!error)
    {
        TransportHelper::processMessages(_connection, httpResponse, &timedOut, &disconnected);
    }
    else
    {
        if(_started)
        {
            Q_EMIT transportStarted(error);
        }
        else
        {
            if(error->getType() == SignalException::ConnectionRefusedError)
            {
                _connection->changeState(Connection::Reconnecting, Connection::Connected);
            }

            if(_connection->ensureReconnecting())
            {
                QLOG_DEBUG() << "LongPollingTranpsort: lost connection...try to reconnect";
                Helper::wait(2);
                //_transport->run();
            }
            else if(_connection->getAutoReconnect())
            {
                QLOG_DEBUG() << "LongPollingTranpsort: (autoconnect=true) lost connection...try to reconnect";
                Helper::wait(2);
                //pollInfo->transport->run();
            }
            else
            {
                _connection->onError(*error);
            }
        }

        delete error;
    }

    if(disconnected)
    {
        _connection->stop();
    }
    else
    {
        mHttpClient->get(_url);
    }

    if(_connection->getState() == Connection::Disconnected)
        return;
}
