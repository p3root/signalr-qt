/*
 *  Copyright (c) 2013, p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at)
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *
 *    Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright notice, this
 *    list of conditions and the following disclaimer in the documentation and/or
 *    other materials provided with the distribution.
 *
 *    Neither the name of the {organization} nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Connection.h"
#include <QString>
#include "Transports/HttpClient.h"
#include "Helper/Helper.h"
#include "Transports/LongPollingTransport.h"

Connection::Connection(const QString host) : _count(0), _keepAliveData(0)
{
    _host = host;
    _state = Disconnected;

    qRegisterMetaType<SignalException>("SignalException");
    _logErrorsToQDebug = false;
}

Connection::~Connection()
{
    if(_keepAliveData)
        delete _keepAliveData;
}


void Connection::start(bool autoReconnect)
{
    start(new HttpClient(this), autoReconnect);
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

const QString &Connection::getConnectionId()
{
    return _connectionId;
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

void Connection::setAdditionalHttpHeaders(QList<QPair<QString, QString> > lst)
{
    _additionalHeaders = lst;
}

void Connection::setAdditionalQueryString(QList<QPair<QString, QString> > lst)
{
    _additionalQueryString = lst;
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

const QString &Connection::getConnectionToken()
{
    return _connectionToken;
}

const QString &Connection::getGroupsToken()
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

KeepAliveData &Connection::getKeepAliveData()
{
    return *_keepAliveData;
}

void Connection::updateLastKeepAlive()
{
    if(_keepAliveData)
        _keepAliveData->setLastKeepAlive(QDateTime::currentDateTime());
}

void Connection::connectionSlow()
{
   Q_EMIT onConnectionSlow();
}

void Connection::stop()
{
    changeState(_state, Disconnected);
    _transport->stop();
    delete _transport;
    _transport = 0;
}

void Connection::negotiateCompleted(const NegotiateResponse* negotiateResponse, SignalException* error)
{
    if(!error)
    {
        if( !(negotiateResponse->protocolVersion == "1.3" || negotiateResponse->protocolVersion == "1.2") )
        {
            onError(SignalException("Invalid protocol version", SignalException::InvalidProtocolVersion));
            stop();
        }
        else
        {
            if(negotiateResponse->keepAliveTimeout > 0)
            {
                _keepAliveData = new KeepAliveData(negotiateResponse->keepAliveTimeout);
            }
            setConnectionState(*negotiateResponse);
            connect(_transport, SIGNAL(transportStarted(SignalException*)), this, SLOT(transportStarted(SignalException*)));
            getTransport()->start("");
        }
    }
    else
    {
        if(getAutoReconnect())
        {
            onError(*error);
            if(_logErrorsToQDebug)
                qDebug() << "Negotation failed, will try it again";
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

