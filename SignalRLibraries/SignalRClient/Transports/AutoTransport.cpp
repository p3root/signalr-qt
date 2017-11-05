#include "AutoTransport.h"

#include "WebSocketTransport.h"
#include "ServerSentEventsTransport.h"
#include "LongPollingTransport.h"
#include "Connection_p.h"

namespace P3 { namespace SignalR { namespace Client {

AutoTransport::AutoTransport() :
    HttpBasedTransport()
{
    _transports = QList<ClientTransport*>();
  //  _transports.append(new WebSocketTransport());
    _transports.append(new ServerSentEventsTransport());
    _transports.append(new LongPollingTransport());

    _index = 0;
    _transport = 0;

}

AutoTransport::~AutoTransport()
{
    _transport = 0;
    qDeleteAll(_transports);
}

void AutoTransport::negotiate()
{
    foreach(ClientTransport *ct, _transports)
    {
        ct->setConnectionPrivate(_connection);
    }

    HttpBasedTransport::negotiate();
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
    _connection->emitLogMessage("Using transport '" + transport->getTransportType() +"'", SignalR::Info);

    disconnect(transport, SIGNAL(transportStarted(QSharedPointer<SignalException>)), this, SLOT(onTransportStated(QSharedPointer<SignalException>)));
    disconnect(transport, SIGNAL(onMessageSentCompleted(QSharedPointer<SignalException>, quint64)), this, SLOT(onMessageSent(QSharedPointer<SignalException>, quint64)));

    connect(transport, SIGNAL(transportStarted(QSharedPointer<SignalException>)), this, SLOT(onTransportStated(QSharedPointer<SignalException>)));
    connect(transport, SIGNAL(onMessageSentCompleted(QSharedPointer<SignalException>, quint64)), this, SLOT(onMessageSent(QSharedPointer<SignalException>, quint64)));
    transport->start(data);

    if(_messages.count() > 0)
    {
        foreach(QString str, _messages)
        {
            transport->send(str);
        }

        _messages.clear();
    }

    _transport = transport;
}

bool AutoTransport::abort(int timeoutMs)
{
    if(_transport)
    {
        return _transport->abort(timeoutMs);
    }
    return true;
}

void AutoTransport::send(QString data)
{
    if(_transport)
        _transport->send(data);
    else
        _messages.append(data);
}

void AutoTransport::lostConnection(ConnectionPrivate *con)
{
    if(_transport)
        _transport->lostConnection(con);
}

void AutoTransport::retry()
{
    if(_transport) {
        _connection->emitLogMessage("Transport '" + _transport->getTransportType() +"' retry connect", SignalR::Info);
        _transport->retry();
    }
}

bool AutoTransport::supportsKeepAlive()
{
    if(_transport)
        return _transport->supportsKeepAlive();
    return false;
}

const QString &AutoTransport::getTransportType()
{
    static QString transport = "autoConnection";
    return transport;
}

void AutoTransport::onTransportStated(QSharedPointer<SignalException> e)
{
    if(!e.isNull())
    {
        if(_index + 1 < _transports.count())
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

void AutoTransport::onMessageSent(QSharedPointer<SignalException> ex, quint64 messageId)
{
    Q_EMIT onMessageSentCompleted(ex, messageId);
}

}}}
