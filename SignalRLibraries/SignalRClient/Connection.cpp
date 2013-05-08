#include "Connection.h"
#include <QString>
#include "Transports/HttpClient.h"
#include "Helper/Helper.h"
#include "Transports/LongPollingTransport.h"
#include <QsLog.h>

Connection::Connection(const QString host) : _count(0)
{
    _host = host;
    _state = Disconnected;

    qRegisterMetaType<SignalException>("SignalException");
}

Connection::~Connection()
{

}


void Connection::start(bool autoReconnect)
{
    start(new HttpClient(), autoReconnect);
}

void Connection::start(HttpClient* client, bool autoReconnect)
{	
    start(new LongPollingTransport(client, this), autoReconnect);
}

void Connection::start(ClientTransport* transport, bool autoReconnect)
{	
    _transport = transport;
    _autoReconnect = autoReconnect;

    if(changeState(Disconnected, Connecting))
    {
        _transport->negotiate();
    }
}

void Connection::send(QString data)
{
    if(_count == sizeof(quint64))
        _count = 0;
    _count++;
    _transport->send(data);
}

Connection::State Connection::getState()
{
    return _state;
}

bool Connection::changeState(State oldState, State newState)
{
    if(_state == oldState)
    {
        _state = newState;

        Q_EMIT stateChanged(oldState, newState);

        return true;
    }

    return false;
}

bool Connection::ensureReconnecting()
{
    changeState(Connected, Reconnecting);

    return _state == Reconnecting;
}

void Connection::setConnectionState(NegotiateResponse negotiateResponse)
{
    _connectionId = negotiateResponse.connectionId;
    _connectionToken = negotiateResponse.connectionToken;
}

QString Connection::onSending()
{
    return "";
}

void Connection::onError(SignalException error)
{
    Q_EMIT errorOccured(error);
}

void Connection::onReceived(QVariant data)
{
    Q_EMIT messageReceived(data);
}

ClientTransport* Connection::getTransport()
{
    return _transport;
}

QString Connection::getUrl()
{
    return _host;
}

QString Connection::getConnectionToken()
{
    return _connectionToken;
}

QString Connection::getGroupsToken()
{
    return _groupsToken;
}

QString Connection::getMessageId()
{
    return _messageId;
}

quint64 Connection::getCount()
{
    return _count;
}

bool Connection::getAutoReconnect()
{
    return _autoReconnect;
}

void Connection::stop()
{
    changeState(_state, Disconnected);
    _transport->stop();
    delete _transport;
}

void Connection::negotiateCompleted(const NegotiateResponse* negotiateResponse, SignalException* error)
{
    if(!error)
    {
        if(negotiateResponse->protocolVersion != "1.2")
        {
            onError(SignalException("Invalid protocol version", SignalException::InvalidProtocolVersion));
            stop();
        }
        else
        {
            setConnectionState(*negotiateResponse);
            connect(_transport, SIGNAL(transportStarted(SignalException*)), this, SLOT(transportStarted(SignalException*)));
            getTransport()->start("");
        }
    }
    else
    {
        if(getAutoReconnect())
        {
            QLOG_DEBUG() << "Negotation failed, will try it again";
            Helper::wait(2);
            getTransport()->negotiate();
        }
        else
        {
            onError(SignalException("Negotiation failed", SignalException::InvalidNegotiationValues));
            stop();
        }
    }
}

void Connection::transportStarted(SignalException* error)
{
    if(!error)
    {
        changeState(Connecting, Connected);
    }
    else
    {
        if(_autoReconnect)
        {
            if(changeState(Disconnected, Connecting))
            {
                _transport->negotiate();
            }
        }
        else
        {

            onError(*error);
            stop();
        }
    }
}

