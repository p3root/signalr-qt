#include "AutoTransport.h"

#include "WebSocketTransport.h"
#include "ServerSentEventsTransport.h"
#include "LongPollingTransport.h"

AutoTransport::AutoTransport(HttpClient *httpClient, Connection *con) : HttpBasedTransport(httpClient, con)
{
    _transports = QList<ClientTransport*>();
    _transports.append(new WebSocketTransport(httpClient, con));
    _transports.append(new ServerSentEventsTransport(httpClient, con));
    _transports.append(new LongPollingTransport(httpClient, con));
    _index = 0;
    _transport = 0;

}

AutoTransport::~AutoTransport()
{
    _transport = 0;
    qDeleteAll(_transports);
}

void AutoTransport::onNegotiatenCompleted(const NegotiateResponse& res)
{
    if(!res.tryWebSockets)
    {
        for(int i = _transports.size() -1; i >= 0; i--)
        {
            if(_transports[i]->getTransportType() == "webSockets")
            {
                _transports.removeAt(i);
            }
        }
    }
}

void AutoTransport::start(QString data)
{
    ClientTransport *transport = _transports[_index];
    connect(transport, SIGNAL(transportStarted(SignalException*)), this, SLOT(onTransportStated(SignalException*)));
    transport->start(data);
}

void AutoTransport::abort()
{
    if(_transport)
    {
        _transport->abort();
    }
}

void AutoTransport::stop()
{
    if(_transport)
    {
        _transport->stop();
    }
}

void AutoTransport::send(QString data)
{
    if(_transport)
        _transport->send(data);
}

const QString &AutoTransport::getTransportType()
{
    static QString transport = "autoConnection";
    return transport;
}

void AutoTransport::onTransportStated(SignalException *e)
{
    if(e)
    {
        if(_index < _transports.count())
        {
            _index++;
            start("");
        }
        else
        {
            Q_EMIT transportStarted(e);
        }
    }
    else
    {
        _transport = _transports[_index];
        Q_EMIT transportStarted(e);
    }
}
