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

#include "ServerSentEventsTransport.h"
#include "Helper/Helper.h"

ServerSentEventsTransport::ServerSentEventsTransport(HttpClient* httpClient, Connection* con) : HttpBasedTransport(httpClient, con), _eventStream(0)
{

}

ServerSentEventsTransport::~ServerSentEventsTransport(void)
{

}

void ServerSentEventsTransport::start(QString)
{
    _url = _connection->getUrl() + "/connect";
    _url += TransportHelper::getReceiveQueryString(_connection, _connection->onSending(), getTransportType());

    QUrl qurl = QUrl(_url);

    if(_eventStream)
        delete _eventStream;

    _eventStream = new HttpEventStream(qurl, _connection->getLogErrorsToQDebug());

    connect(_eventStream, SIGNAL(packetReady(QString,SignalException*)), this, SLOT(packetReceived(QString,SignalException*)));
    connect(_eventStream, SIGNAL(connected(SignalException*)), this, SLOT(connected(SignalException*)));

    _eventStream->start();

}

void ServerSentEventsTransport::abort()
{
    _eventStream->close();
    _eventStream->deleteLater();
}

void ServerSentEventsTransport::stop()
{
    _eventStream->close();
}

void ServerSentEventsTransport::lostConnection(Connection *)
{
    reconnect();
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
        else
        {
            // Reconnected
            _connection->changeState(Connection::Reconnecting, Connection::Connected);
        }
    }
    else if(error != 0)
    {
        if(error->getType() == SignalException::EventStreamSocketLost)
        {
            reconnect();
        }
        else if(_connection->ensureReconnecting())
        {
            if(_connection->getLogErrorsToQDebug())
                qDebug() << "ServerSentEventsTransport: Lost connection...try to reconnect";

            //wait to seconds before reconnecting
            Helper::wait(2);
            reconnect();
        }
        else if(_connection->getAutoReconnect())
        {
            if(_connection->getLogErrorsToQDebug())
                qDebug() << "ServerSentEventsTransport: (autoconnect=true)  Lost connection...try to reconnect";


            //wait to seconds before reconnecting
            Helper::wait(2);
            reconnect();
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
        _connection->getKeepAliveData().setLastKeepAlive(QDateTime::currentDateTime());
        TransportHelper::processMessages(_connection, data, &timedOut, &disconnected);
    }

    if(disconnected)
    {
        _connection->stop();
    }
}

void ServerSentEventsTransport::connected(SignalException *error)
{
    Q_EMIT transportStarted(error);
}

void ServerSentEventsTransport::reconnect()
{
    _eventStream->close();
    _eventStream->start();
}
