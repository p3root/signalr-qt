/*
 *  Copyright (c) 2013-2014, p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at)
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

#include "Connection_p.h"
#include "Connection.h"

#include "Transports/HttpClient.h"
#include "Helper/Helper.h"
#include "Transports/AutoTransport.h"

namespace P3 { namespace SignalR { namespace Client {

ConnectionPrivate::ConnectionPrivate(const QString &host, Connection *connection, QObject *parent) :
    QObject(parent),
    _transport(0),
    _count(0),
    _keepAliveData(0),
    q_ptr(connection)
{
    _host = host;
    _state = SignalR::Disconnected;

    qRegisterMetaType<SignalException>("SignalException");
    qRegisterMetaType<SignalR::State>("State");
    qRegisterMetaType<SignalR::State>("Connection::State");
    _reconnectWaitTime = 5;

#ifndef QT_NO_SSL
    _sslConfiguration = QSslConfiguration::defaultConfiguration();
#endif

    connect(this, SIGNAL(sendData(QString)), this, SLOT(onSendData(QString)));
}


ConnectionPrivate::~ConnectionPrivate()
{

}

void ConnectionPrivate::start(bool autoReconnect)
{
    ClientTransport *ct = new AutoTransport();
    ct->setConnectionPrivate(this);
    start(ct, autoReconnect);
}

void ConnectionPrivate::start(ClientTransport* transport, bool autoReconnect)
{
    transport->setConnectionPrivate(this);
    _transport = transport;
    _autoReconnect = autoReconnect;

    connect(transport, SIGNAL(onMessageSentCompleted(SignalException*)), this, SLOT(transportMessageSent(SignalException*)));

    if(changeState(SignalR::Disconnected, SignalR::Connecting))
    {
        _transport->negotiate();
    }
}

void ConnectionPrivate::send(const QString &data)
{
    Q_EMIT sendData(data);
}

void ConnectionPrivate::retry()
{
    _transport->retry();
}

SignalR::State ConnectionPrivate::getState()
{
    return _state;
}

const QString &ConnectionPrivate::getConnectionId() const
{
    return _connectionId;
}

bool ConnectionPrivate::changeState(SignalR::State oldState, SignalR::State newState)
{
    Q_Q(Connection);
    q->onStateChanged(oldState, newState);

    if(_state == oldState)
    {
        _state = newState;

        Q_EMIT q->stateChanged(oldState, newState);

        return true;
    }

    return false;

}

bool ConnectionPrivate::ensureReconnecting()
{
    changeState(SignalR::Connected, SignalR::Reconnecting);

    return _state == SignalR::Reconnecting;
}

void ConnectionPrivate::setConnectionState(NegotiateResponse negotiateResponse)
{
    _connectionId = negotiateResponse.connectionId;
    _connectionToken = negotiateResponse.connectionToken;
}

QString ConnectionPrivate::onSending()
{
    return q_ptr->onSending();
}

void ConnectionPrivate::setAdditionalHttpHeaders(QList<QPair<QString, QString> > lst)
{
    _additionalHeaders = lst;
}

void ConnectionPrivate::setAdditionalQueryString(QList<QPair<QString, QString> > lst)
{
    _additionalQueryString = lst;
}

void ConnectionPrivate::onError(SignalException error)
{
    Q_Q(Connection);
    Q_EMIT q->errorOccured(error);
}

void ConnectionPrivate::onReceived(QVariant &data)
{
    Q_Q(Connection);
    Q_EMIT q->onReceived(data);
}

const ClientTransport *ConnectionPrivate::getTransport() const
{
    return _transport;
}

const QString &ConnectionPrivate::getUrl() const
{
    return _host;
}

const QString &ConnectionPrivate::getConnectionToken() const
{
    return _connectionToken;
}

const QString &ConnectionPrivate::getGroupsToken() const
{
    return _groupsToken;
}

const QString &ConnectionPrivate::getMessageId() const
{
    return _messageId;

}

quint64 ConnectionPrivate::getNextCount()
{
    return ++_count;
}

void ConnectionPrivate::presetCount(quint64 preset)
{
    _count = preset;
}

bool ConnectionPrivate::getAutoReconnect() const
{
    return _autoReconnect;
}

KeepAliveData &ConnectionPrivate::getKeepAliveData()
{
    return *_keepAliveData;
}

void ConnectionPrivate::updateLastKeepAlive()
{
    if(_keepAliveData)
        _keepAliveData->setLastKeepAlive(QDateTime::currentDateTimeUtc());
}

void ConnectionPrivate::connectionSlow()
{
    Q_Q(Connection);
    Q_EMIT q->onConnectionSlow();
}

bool ConnectionPrivate::stop(int timeoutMs)
{
    changeState(_state, SignalR::Disconnecting);
    bool abort = _transport->abort(timeoutMs);
    _transport->deleteLater();
    _transport = 0;
    changeState(_state, SignalR::Disconnected);

    _connectionId = "";
    _connectionToken = "";

    return abort;
}

void ConnectionPrivate::negotiateCompleted(const NegotiateResponse* negotiateResponse)
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
        _transport->start("");
    }
}

void ConnectionPrivate::emitLogMessage(QString msg, SignalR::LogSeverity severity)
{
    Q_Q(Connection);
    Q_EMIT q->logMessage(msg, severity);
}

HeartbeatMonitor *ConnectionPrivate::createHeartbeatMonitor()
{
    return new HeartbeatMonitor(this);
}

void ConnectionPrivate::onSendData(const QString &data)
{
    _transport->send(data);
}

void ConnectionPrivate::transportStarted(SignalException* error)
{
    Q_Q(Connection);
    if(!error)
    {
        if(_state == SignalR::Reconnecting)
            changeState(SignalR::Reconnecting, SignalR::Connected);
        else
            changeState(SignalR::Connecting, SignalR::Connected);
    }
    else
    {
        if(_autoReconnect)
        {
            if(changeState(SignalR::Disconnected, SignalR::Connecting))
            {
                _transport->negotiate();
            }
            else if(changeState(SignalR::Connected, SignalR::Reconnecting))
            {
                _transport->start("");
            }
            else if(changeState(SignalR::Reconnecting, SignalR::Connecting))
            {
                _transport->start("");
            }
            else if(changeState(SignalR::Connecting, SignalR::Reconnecting))
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

    q->onTransportStarted(error);
}

void ConnectionPrivate::transportMessageSent(SignalException *ex)
{
    Q_Q(Connection);
    q->onMessageSentCompleted(ex);
    Q_EMIT q->messageSentCompleted(ex);
}


}}}
