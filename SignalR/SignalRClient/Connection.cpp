#include "Connection.h"
#include "ConnectionHandler.h"
#include <QString>
#include "Transports/DefaultHttpClient.h"
#include "Transports/AutoTransport.h"
#include "Helper/Helper.h"

Connection::Connection(const QString host, ConnectionHandler* handler) : _count(0)
{
    _host = host;
    _state = Disconnected;
    _handler = handler;
}

Connection::~Connection()
{

}


void Connection::start(bool autoReconnect)
{
     start(new DefaultHttpClient(), autoReconnect);
}

void Connection::start(HttpClient* client, bool autoReconnect)
{	
    start(new AutoTransport(client), autoReconnect);
}

void Connection::start(ClientTransport* transport, bool autoReconnect)
{	
    _transport = transport;
    _autoReconnect = autoReconnect;

    if(changeState(Disconnected, Connecting))
    {
        _transport->negotiate(this, &Connection::onNegotiateCompleted, this);
    }
}

void Connection::send(QString data)
{
    if(_count == sizeof(quint64))
        _count = 0;
    _count++;
    _transport->send(this, data);
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

        _handler->onStateChanged(oldState, newState);

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
    _handler->onError(error);
}

void Connection::onReceived(QVariant data)
{
    _handler->receivedData(data);
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
    _transport->stop(this);
    delete _transport;
}

void Connection::onNegotiateCompleted(NegotiateResponse* negotiateResponse, SignalException* error, void* state)
{	
    Connection* connection = (Connection*)state;

    if(!error)
    {
        if(negotiateResponse->protocolVersion != "1.2")
        {
            connection->onError(SignalException("Invalid protocol version", SignalException::InvalidProtocolVersion));
            connection->stop();
        }
        else
        {
            connection->setConnectionState(*negotiateResponse);
            connection->getTransport()->start(connection, Connection::onTransportStartCompleted, "", connection);
        }
    }
    else 
    {
        if(connection->getAutoReconnect())
        {
            QLOG_DEBUG() << "Negotation failed, will try it again";
            Helper::wait(2);
            connection->getTransport()->negotiate(connection, &Connection::onNegotiateCompleted, connection);
        }
        else
        {
            connection->onError(SignalException("Negotiation failed", SignalException::InvalidNegotiationValues));
            connection->stop();
        }
    }
}

void Connection::onTransportStartCompleted(SignalException* error, void* state)
{
    Connection* connection = (Connection*)state;

    if(!error)
    {
        connection->changeState(Connecting, Connected);
    }
    else 
    {
        connection->onError(*error);
        connection->stop();
    }
}

