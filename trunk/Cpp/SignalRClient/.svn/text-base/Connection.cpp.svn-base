#include "Connection.h"
#include "ConnectionHandler.h"
#include <QString>
#include "Transports/DefaultHttpClient.h"
#include "Transports/AutoTransport.h"

Connection::Connection(const QString host,ConnectionHandler* handler) : _count(0)
{
    _host = host;
    _state = Disconnected;
    _handler = handler;
}

Connection::~Connection()
{

}


void Connection::start()
{
     start(new DefaultHttpClient());
}

void Connection::start(HttpClient* client)
{	
    start(new AutoTransport(client));
}

void Connection::start(ClientTransport* transport)
{	
    _transport = transport;

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

bool Connection::changeState(State oldState, State newState)
{
    if(_state == oldState)
    {
        _state = newState;

        _handler->onStateChanged(oldState, oldState);

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

void Connection::onReceived(QString data)
{
    _handler->onReceived(data);
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
        connection->onError(SignalException("Negotiation failed", SignalException::InvalidNegotiationValues));
        connection->stop();
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


quint64 Connection::getCount()
{
    return _count;
}
