/*
 *  Copyright (c) 2013, p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at)
 *  All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. All advertising materials mentioning features or use of this software
 *      must display the following acknowledgement:
 *      This product includes software developed by the p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at).
 *   4. Neither the name of the p3root - Patrik Pfaffenbauer nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY p3root - Patrik Pfaffenbauer ''AS IS'' AND ANY
 *   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *   DISCLAIMED. IN NO EVENT SHALL p3root - Patrik Pfaffenbauer BE LIABLE FOR ANY
 *   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Client.h"

#include <QCoreApplication>


Client::Client(QCoreApplication &app)
{
    _timer.setSingleShot(true);
    _timer.setInterval(30000);
   // _timer.start();
    connect(&_timer, SIGNAL(timeout()), this, SLOT(timerTick()));
    connect(&app, SIGNAL(aboutToQuit()), SLOT(stop()));
}

Client::~Client()
{
    delete _connection;
    delete _client;
}

void Client::start()
{
    QLOG_DEBUG() << "Client Thread: " << thread()->currentThreadId();
    _connection = new HubConnection("http://192.168.0.202:8080/signalr");

    _client = new HttpClient();
    _transport = new LongPollingTransport(_client, _connection);

    HubProxy* proxy = _connection->createHubProxy("Chat");

    connect(proxy, SIGNAL(hubMessageReceived(QVariant)), this, SLOT(onHubMessageReceived(QVariant)));

    connect(_connection, SIGNAL(errorOccured(SignalException)), this, SLOT(onError(SignalException)));
    connect(_connection, SIGNAL(stateChanged(Connection::State,Connection::State)), this, SLOT(onStateChanged(Connection::State,Connection::State)));

    _connection->start(_transport, true);
}

void Client::stop()
{
    _connection->stop();
}

void Client::onHubMessageReceived(QVariant v)
{
    QLOG_DEBUG() << v;
}

void Client::onError(SignalException error)
{
     QLOG_DEBUG() << error.what();
}

void Client::onStateChanged(Connection::State oldState, Connection::State newState)
{
    QLOG_DEBUG()  << "state changed: " << oldState << " -> " << newState;

    if(newState == Connection::Connected)
    {
        HubCallback* callback = new HubCallback(0);
        connect(callback, SIGNAL(messageReceived(HubCallback*,QVariant)), this, SLOT(answerReceived(HubCallback*,QVariant)));
        HubProxy* prox = _connection->getByName("Chat");
        prox->invoke("send", "test", callback);
    }
}

void Client::answerReceived(HubCallback *c, QVariant v)
{



    delete c; //VERY IMPORTANT, otherwise the callback will not be delete -> memory leak
}

void Client::timerTick()
{
    qApp->exit(3);
}

