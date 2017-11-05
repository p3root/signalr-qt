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
#include <QSharedPointer>
#include <QThread>

namespace P3 { namespace SignalR { namespace Client {

ConnectionPrivate::ConnectionPrivate(const QString &host, Connection *connection, QObject *parent) :
    QObject(parent),
    _transport(0),
    _count(0),
    _keepAliveData(0),
    _stateLocker(QMutex::Recursive),
    q_ptr(connection)
{
    _host = host;
    _state = SignalR::Disconnected;
    _messageIdLocker = new QMutex(QMutex::Recursive);

    qRegisterMetaType<SignalException>("SignalException");
    qRegisterMetaType<SignalR::State>("State");
    qRegisterMetaType<SignalR::State>("SignalR::State");
    qRegisterMetaType<QSharedPointer<SignalException> >("QSharedPointer<SignalException>");
    _reconnectWaitTime = 5000;
    _postTimeoutMs = 60*3*1000; //3 min

    _messageRepeatReconAmount = -1;
#ifndef QT_NO_SSL
    _sslConfiguration = QSslConfiguration::defaultConfiguration();
#endif

    connect(this, SIGNAL(sendData(QString)), this, SLOT(onSendData(QString)));
    connect(this, SIGNAL(startRetry()), this, SLOT(onRetry()));

    _tid = "";
}


ConnectionPrivate::~ConnectionPrivate()
{
    delete _keepAliveData;
    delete _messageIdLocker;
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

    _monitor = new HeartbeatMonitor(this);

    connect(transport, SIGNAL(onMessageSentCompleted(QSharedPointer<SignalException>, quint64)), this, SLOT(transportMessageSent(QSharedPointer<SignalException>, quint64)));

    if(_state == SignalR::Disconnected)
    {
        changeState(SignalR::Disconnected, SignalR::Connecting);
        _transport->negotiate();
    }
}

void ConnectionPrivate::send(const QString &data)
{
    if(!_transport)
    {
        emitLogMessage("Could not send data. Transport is not initialized", SignalR::Error);
        return;
    }
    if(QThread::currentThreadId() != _transport->_threadId)
    {
        Q_EMIT sendData(data);
    }
    else
    {
        _transport->send(data);
    }
}

void ConnectionPrivate::retry()
{
    if(!_transport)
    {
        emitLogMessage("Could not retry connection. Transport is not initialized", SignalR::Error);
        return;
    }

    if(QThread::currentThreadId() != _transport->_threadId)
    {
        Q_EMIT startRetry();
    }
    else
    {
        _transport->retry();
    }
}

SignalR::State ConnectionPrivate::getState()
{
    QMutexLocker l(&_stateLocker);
    return _state;
}

const QString &ConnectionPrivate::getConnectionId() const
{
    return _connectionId;
}

void ConnectionPrivate::changeState(SignalR::State oldState, SignalR::State newState)
{
    Q_Q(Connection);
    QMutexLocker l(&_stateLocker);

    SignalR::State currentState = _state;
    _state = newState;

    emitLogMessage(QString("Change state from %1 to %2").arg(translateState(oldState), translateState(newState)), SignalR::Debug);

    if(currentState != newState)
    {
        Q_EMIT q->stateChanged(oldState, newState);
        q->onStateChanged(oldState, newState);
    }
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

void ConnectionPrivate::onError(QSharedPointer<SignalException> error)
{
    Q_Q(Connection);
    Q_EMIT q->errorOccured(error);
}

void ConnectionPrivate::onReceived(QVariant &data)
{
    Q_Q(Connection);
    q->onReceived(data);
}

void ConnectionPrivate::setGroupsToken(const QString &token)
{
    _groupsToken = token;
}

void ConnectionPrivate::setMessageId(const QString &messageId)
{
    QMutexLocker l(_messageIdLocker);
    _messageId = messageId;
}

ClientTransport *ConnectionPrivate::getTransport()
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
    QMutexLocker l(_messageIdLocker);
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

KeepAliveData *ConnectionPrivate::getKeepAliveData()
{
    return _keepAliveData;
}

void ConnectionPrivate::updateLastKeepAlive()
{
    if(_keepAliveData)
        _keepAliveData->setLastKeepAlive(QDateTime::currentDateTimeUtc());

    Q_Q(Connection);
    Q_EMIT q->keepAliveReceived();
}

void ConnectionPrivate::updateLastRetryTime()
{
    _lastRetry = QDateTime::currentDateTimeUtc();

    Q_Q(Connection);
    Q_EMIT q->retryReceived();
}

void ConnectionPrivate::connectionSlow()
{
    Q_Q(Connection);
    Q_EMIT q->onConnectionSlow();
}

bool ConnectionPrivate::stop(int timeoutMs)
{
    if(_state == SignalR::Disconnected || _state == SignalR::Disconnecting)
        return true;

    QMutexLocker l(&_stateLocker);

    changeState(_state, SignalR::Disconnecting);
    bool abort = _transport->abort(timeoutMs);
    _transport->deleteLater();
    _transport = 0;

    _connectionId = "";
    _connectionToken = "";
    _messageId = "";

    _monitor->stop();

    changeState(_state, SignalR::Disconnected);

    return abort;
}

void ConnectionPrivate::negotiateCompleted(const NegotiateResponse* negotiateResponse)
{

    if( !(negotiateResponse->protocolVersion == "1.3" || negotiateResponse->protocolVersion == "1.2" || negotiateResponse->protocolVersion == "1.5"))
    {
        QSharedPointer<SignalException> invalidProtocol = QSharedPointer<SignalException>(new SignalException("Invalid protocol version", SignalException::InvalidProtocolVersion));
        onError(invalidProtocol);
        stop();
    }
    else
    {
        if(negotiateResponse->keepAliveTimeout > 0)
        {
            delete _keepAliveData;
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
        connect(_transport, SIGNAL(transportStarted(QSharedPointer<SignalException>)), this, SLOT(transportStarted(QSharedPointer<SignalException>)), Qt::UniqueConnection);
        _transport->start("");
    }
}

void ConnectionPrivate::emitLogMessage(QString msg, SignalR::LogSeverity severity)
{
    Q_Q(Connection);
    Q_EMIT q->logMessage(msg, severity);
}

HeartbeatMonitor &ConnectionPrivate::getHeartbeatMonitor()
{
    return *_monitor;
}

QString ConnectionPrivate::translateState(SignalR::State state)
{
    switch(state) {
        case SignalR::Connecting:
            return "Connecting";
        case SignalR::Connected:
            return "Connected";
        case SignalR::Reconnecting:
            return "Reconnecting";
        case SignalR::Disconnecting:
            return "Disconnecting";
        case SignalR::Disconnected:
            return "Disconnected";
    }
    return "N/A";
}

void ConnectionPrivate::onSendData(const QString data)
{
    _transport->send(data);
}

void ConnectionPrivate::onRetry()
{
    _transport->retry();
}

void ConnectionPrivate::transportStarted(QSharedPointer<SignalException> error)
{
    Q_Q(Connection);
    if(error.isNull())
    {
        if(_state == SignalR::Reconnecting)
            changeState(SignalR::Reconnecting, SignalR::Connected);
        else
            changeState(SignalR::Connecting, SignalR::Connected);

        if(_transport && _transport->supportsKeepAlive())
            _monitor->start();
    }
    else
    {
        if(_autoReconnect)
        {
            if(getState() == SignalR::Disconnected)
            {
                changeState(SignalR::Disconnected, SignalR::Connecting);
                _transport->negotiate();
            }
            else if(getState() == SignalR::Connected)
            {
                changeState(SignalR::Connected, SignalR::Reconnecting);
                _transport->start("");
            }
            else if(getState() == SignalR::Reconnecting)
            {
                _transport->start("");
            }
            else if(getState() == SignalR::Connecting)
            {
                _transport->start("");
            }
        }
        else
        {

            onError(error);
            stop();
        }
    }

    q->onTransportStarted(error);
}

void ConnectionPrivate::transportMessageSent(QSharedPointer<SignalException> ex, quint64 messageId)
{
    Q_Q(Connection);
    q->onMessageSentCompleted(ex, messageId);
    Q_EMIT q->messageSentCompleted(ex);
}


}}}
