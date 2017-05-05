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

#include "ServerSentEventsTransport.h"
#include "Helper/Helper.h"
#include "Connection.h"
#include "Connection_p.h"
#include "Helper/TransportHelper.h"

namespace P3 { namespace SignalR { namespace Client {

ServerSentEventsTransport::ServerSentEventsTransport() :
    HttpBasedTransport(),
    _eventStream(0)
{
    _started = false;
}

ServerSentEventsTransport::~ServerSentEventsTransport(void)
{

}

void ServerSentEventsTransport::start(QString)
{
    if(_eventStream)
    {
        _eventStream->deleteLater();
        _eventStream = 0;
    }

    QString urlAppend = "connect";
    if(_started)
        urlAppend = "reconnect";
    _url = _connection->getUrl() + "/"+urlAppend;

    _url += TransportHelper::getReceiveQueryString(_connection, getTransportType());

    startEventStream();
}

bool ServerSentEventsTransport::abort(int timeoutMs)
{
    if(_eventStream) {
        _eventStream->close();
        _eventStream->deleteLater();
        _eventStream = 0;
    }

    return HttpBasedTransport::abort(timeoutMs);
}

void ServerSentEventsTransport::stop()
{
    if(_eventStream)
    {
        _eventStream->close();
        _eventStream->deleteLater();
        _eventStream = 0;
    }
}

void ServerSentEventsTransport::lostConnection(ConnectionPrivate *con)
{
    _connection->changeState(SignalR::Connected, SignalR::Reconnecting);
    HttpBasedTransport::lostConnection(con);
    retry();
}


void ServerSentEventsTransport::retry()
{
    disconnect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
    _retryTimerTimeout.stop();

    if(_eventStream)
    {
        _eventStream->close();
        _eventStream->deleteLater();
        _eventStream = 0;
    }
    startEventStream();
}

const QString &ServerSentEventsTransport::getTransportType()
{
    static QString type = "serverSentEvents";
    return type;
}

void ServerSentEventsTransport::packetReceived(QString data, QSharedPointer<SignalException> error)
{
    bool timedOut = false, disconnected = false;
    _lastSignalException = error;

    if(data == "data: initialized")
    {
        _connection->changeState(_connection->getState(), SignalR::Connected);

    }
    else if(data.startsWith("id:"))
    {
        //ignore the event
    }
    else if(!error.isNull())
    {
        if(_connection->ensureReconnecting())
        {
            _connection->onError(error);

            connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
            _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime());
            _retryTimerTimeout.start();

            logReconnectMessage();
            return;
        }
        else if(_connection->getAutoReconnect())
        {
            _connection->onError(error);

            connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
            _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime());
            _retryTimerTimeout.start();

            logReconnectMessage();
            return;
        }
        else
        {
            _connection->onError(error);
        }
    }
    else
    {
        data = data.remove(0, data.indexOf("data: ")+5);

        _connection->changeState(_connection->getState(), SignalR::Connected);
        _connection->updateLastKeepAlive();

        _connection->emitLogMessage("SSE: Message received", SignalR::Debug);
        QSharedPointer<SignalException> e = TransportHelper::processMessages(_connection, data, &timedOut, &disconnected);

        if(!e.isNull())
        {
            _connection->onError(e);
        }
    }

    if(disconnected)
    {
        _connection->emitLogMessage("SSE: SignalR Server said we should disconnect, and open a new connection if we want", SignalR::Info);
        _connection->stop(2000);
    }
    else if(timedOut)
    {
        _connection->emitLogMessage("SSE: SignalR timed out, we should reconnect", SignalR::Info);
        _connection->changeState(_connection->getState(), SignalR::Reconnecting);
        start("");
    }
}

void ServerSentEventsTransport::connected(QSharedPointer<SignalException> error)
{
    if(!_started)
    {
        _connection->updateLastKeepAlive();

        if(!_started)
            Q_EMIT transportStarted(error);

        _started = true;
    }
    else if(_started && error.isNull()) {
        _retryTimerTimeout.stop();
    }
    else if(!error.isNull())
    {
        _connection->onError(error);

        connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
        _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime());
        _retryTimerTimeout.start();

        logReconnectMessage();

        return;
    }

    if(error.isNull())
    {
        disconnect(_eventStream, SIGNAL(packetReady(QString,QSharedPointer<SignalException>)), this, SLOT(packetReceived(QString,QSharedPointer<SignalException>)));
        connect(_eventStream, SIGNAL(packetReady(QString,QSharedPointer<SignalException>)), this, SLOT(packetReceived(QString,QSharedPointer<SignalException>)));
    }
}

void ServerSentEventsTransport::reconnectTimerTick()
{
    _retryTimerTimeout.stop();
    disconnect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
    closeEventStream();

    _connection->changeState(_connection->getState(), SignalR::Reconnecting);

    start("");
}

void ServerSentEventsTransport::restartConnection()
{
    _connection->stop(1000);
}

void ServerSentEventsTransport::closeEventStream()
{
    if(!_eventStream)
        return;

    _eventStream->close();
    _eventStream->deleteLater();
    _eventStream = 0;
}

void ServerSentEventsTransport::startEventStream()
{
    _connection->updateLastRetryTime();
    closeEventStream();

    _eventStream = new HttpEventStream(_url,_connection, this);

    connect(_eventStream, SIGNAL(connected(QSharedPointer<SignalException>)), this, SLOT(connected(QSharedPointer<SignalException>)));
    connect(_eventStream, SIGNAL(restartConnection()), this, SLOT(restartConnection()));
    _eventStream->open();

}

void ServerSentEventsTransport::logReconnectMessage()
{
    _connection->emitLogMessage("SSE: Lost connection, try to reconnect in " + QString::number(_connection->getReconnectWaitTime()) + "ms", SignalR::Debug);

}

}}}
