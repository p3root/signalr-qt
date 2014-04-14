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

    _url += TransportHelper::getReceiveQueryString(_connection, _connection->onSending(), getTransportType());

    startEventStream();
}

void ServerSentEventsTransport::abort()
{
    _eventStream->close();
    _eventStream->deleteLater();
    _eventStream = 0;
}

void ServerSentEventsTransport::stop()
{
    _eventStream->close();
}

void ServerSentEventsTransport::lostConnection(Connection *)
{
    // reconnect();
}

void ServerSentEventsTransport::retry()
{
    stop();
    startEventStream();
}

const QString &ServerSentEventsTransport::getTransportType()
{
    static QString type = "serverSentEvents";
    return type;
}

void ServerSentEventsTransport::packetReceived(QString data, SignalException *error)
{
    bool timedOut = false, disconnected = false;

    if(data == "data: initialized")
    {
        if(_started)
        {
            Q_EMIT transportStarted(0);
        }
        _connection->changeState(_connection->getState(), SignalR::Connected);

    }
    else if(error != 0)
    {
        if(_connection->ensureReconnecting())
        {
            connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
            _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime() * 1000);
            _retryTimerTimeout.start();

            if(_connection->getState() == SignalR::Connected)
                _connection->changeState(_connection->getState(), SignalR::Reconnecting);

            logReconnectMessage();
            return;
        }
        else if(_connection->getAutoReconnect())
        {
            connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
            _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime() * 1000);
            _retryTimerTimeout.start();

            if(_connection->getState() == SignalR::Connected)
                _connection->changeState(_connection->getState(), SignalR::Reconnecting);

            logReconnectMessage();
            return;
        }
        else
        {
            _connection->onError(*error);
        }

        delete error;
    }
    else
    {
        data = data.remove(0, data.indexOf("data: ")+5);
        _connection->getKeepAliveData().setLastKeepAlive(QDateTime::currentDateTimeUtc());

        _connection->emitLogMessage("SSE: KeepAlive received", SignalR::Debug);
        SignalException *e = TransportHelper::processMessages(_connection, data, &timedOut, &disconnected);

        if(e)
        {
            _connection->onError(*e);
        }
    }

    if(disconnected)
    {
        _connection->stop();
    }
}

void ServerSentEventsTransport::connected(SignalException *error)
{
    if(!_started)
    {
        _started = true;
        Q_EMIT transportStarted(error);
    }
    else if(error)
    {
        connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
        _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime() * 1000);
        _retryTimerTimeout.start();

        _connection->onError(*error);

        logReconnectMessage();

        return;
    }

    if(!error)
    {
        connect(_eventStream, SIGNAL(packetReady(QString,SignalException*)), this, SLOT(packetReceived(QString,SignalException*)));
    }
}

void ServerSentEventsTransport::reconnectTimerTick()
{
    _retryTimerTimeout.stop();
    disconnect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectTimerTick()));
    closeEventStream();
    start("");
}

void ServerSentEventsTransport::closeEventStream()
{
    if(!_eventStream)
        return;

    if(_eventStream->isRunning())
        _eventStream->close();
    _eventStream->deleteLater();
    _eventStream = 0;
}

void ServerSentEventsTransport::startEventStream()
{
    closeEventStream();

    _eventStream = new HttpEventStream(_url,_connection);

    connect(_eventStream, SIGNAL(connected(SignalException*)), this, SLOT(connected(SignalException*)));

    _eventStream->start();

}

void ServerSentEventsTransport::logReconnectMessage()
{
    _connection->emitLogMessage("SSE: Lost connection, try to reconnect in " + QString::number(_connection->getReconnectWaitTime()) + "s", SignalR::Debug);

}

}}}
