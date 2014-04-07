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
#include "Transports/AutoTransport.h"

namespace P3 { namespace SignalR { namespace Client {

Connection::Connection(const QString &host) : _transport(0), _count(0), _keepAliveData(0)
{
    _host = host;
    _state = Disconnected;

    qRegisterMetaType<SignalException>("SignalException");
    qRegisterMetaType<State>("State");
    qRegisterMetaType<Connection::State>("Connection::State");
    _logErrorsToQDebug = false;
    _reconnectWaitTime = 5;
}

Connection::~Connection()
{
    delete _keepAliveData;
}

void Connection::start(bool autoReconnect)
{
    start(new HttpClient(this), autoReconnect);
}

void Connection::start(HttpClient* client, bool autoReconnect)
{	
    start(new AutoTransport(client, this), autoReconnect);
}

void Connection::start(ClientTransport* transport, bool autoReconnect)
{	
    _transport = transport;
    _autoReconnect = autoReconnect;

    connect(transport, SIGNAL(onMessageSentCompleted(SignalException*)), this, SLOT(transportMessageSent(SignalException*)));

    if(changeState(Disconnected, Connecting))
    {
        _transport->negotiate();
    }
}

void Connection::send(const QString &data)
{
    _count++;
    _transport->send(data);
}

void Connection::retry()
{
    _transport->retry();
}

Connection::State Connection::getState()
{
    return _state;
}

const QString &Connection::getConnectionId() const
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

const QString &Connection::getUrl() const
{
    return _host;
}

const QString &Connection::getConnectionToken() const
{
    return _connectionToken;
}

const QString &Connection::getGroupsToken() const
{
    return _groupsToken;
}

const QString &Connection::getMessageId() const
{
    return _messageId;
}

quint64 Connection::getCount() const
{
    return _count;
}

bool Connection::getAutoReconnect() const
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
        _keepAliveData->setLastKeepAlive(QDateTime::currentDateTimeUtc());
}

void Connection::connectionSlow()
{
    Q_EMIT onConnectionSlow();
}

bool Connection::stop(int timeoutMs)
{
    changeState(_state, Disconnecting);
    bool abort = _transport->abort(timeoutMs);
    _transport->deleteLater();
    _transport = 0;
    changeState(_state, Disconnected);

    _connectionId = "";
    _connectionToken = "";

    return abort;
}

void Connection::negotiateCompleted(const NegotiateResponse* negotiateResponse)
{

    if( !(negotiateResponse->protocolVersion == "1.3" || negotiateResponse->protocolVersion == "1.2"))
    {
        onError(SignalException("Invalid protocol version", SignalException::InvalidProtocolVersion));
        stop();
    }
    else
    {
        if(negotiateResponse->keepAliveTimeout > 0)
        {
            _keepAliveData = new KeepAliveData(negotiateResponse->keepAliveTimeout, negotiateResponse->transportConnectTimeout);
        }
        setConnectionState(*negotiateResponse);
        _tryWebSockets = negotiateResponse->tryWebSockets;
        if(negotiateResponse->webSocketsUrl.isEmpty())
            _webSocketsUrl = _host;
        else
            _webSocketsUrl = negotiateResponse->webSocketsUrl;
        _protocolVersion = negotiateResponse->protocolVersion;
        // disconnect(this, SLOT(transportStarted(SignalException*)));
        connect(_transport, SIGNAL(transportStarted(SignalException*)), this, SLOT(transportStarted(SignalException*)), Qt::UniqueConnection);
        getTransport()->start("");
    }
}

void Connection::emitLogMessage(QString msg, Connection::LogSeverity severity)
{
    Q_EMIT logMessage(msg, severity);
}

void Connection::transportStarted(SignalException* error)
{
    if(!error)
    {
        if(_state == Reconnecting)
            changeState(Reconnecting, Connected);
        else
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
            else if(changeState(Connected, Reconnecting))
            {
                _transport->start("");
            }
            else if(changeState(Reconnecting, Connecting))
            {
                _transport->start("");
            }
        }
        else
        {

            onError(*error);
            stop();
        }
    }

    onTransportStarted(error);
}

void Connection::transportMessageSent(SignalException *ex)
{
    onMessageSentCompleted(ex);
    Q_EMIT messageSentCompleted(ex);
}

}}}
