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
    qDebug() << "Client Thread: " << thread()->currentThreadId();
    _connection = new HubConnection("http://sentiint:9085/Services.Push/signalr");

    Connection::UserCredentials uc;
    uc.username = "alex2";
    uc.password = "QTWqncG4QqZT3qhGkD3blb+4xaEMUEp/oW4QvDHR/fA="; //application specified
    uc.authorizationMethod = "Basic";
    _connection->setCredentials(uc);

    _monitor = new HeartbeatMonitor(_connection, 0);

    _client = new HttpClient();
    _transport = new LongPollingTransport(_client, _connection);

    HubProxy* proxy = _connection->createHubProxy("WriteHub");

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
    qDebug() << v;

    HubProxy* prox = _connection->getByName("WriteHub");
    QMap<QString, QVariant> map = v.toMap();
    HubCallback* callback = new HubCallback(0);
    connect(callback, SIGNAL(messageReceived(HubCallback*,QVariant)), this, SLOT(answerReceived(HubCallback*,QVariant)));

    prox->invoke("ConfigReceived",  map["A"].toStringList(), callback);

}

void Client::onError(SignalException error)
{
     qDebug() << error.what();
}

void Client::onStateChanged(Connection::State oldState, Connection::State newState)
{
    qDebug()  << "state changed: " << oldState << " -> " << newState;

    if(newState == Connection::Connected)
    {
        _monitor->start();
        HubCallback* callback = new HubCallback(0);
        connect(callback, SIGNAL(messageReceived(HubCallback*,QVariant)), this, SLOT(answerReceived(HubCallback*,QVariant)));
        HubProxy* prox = _connection->getByName("WriteHub");
        prox->invoke("JoinGroup", "110.110", callback);
    }
}

void Client::answerReceived(HubCallback *c, QVariant v)
{
    Q_UNUSED(v);
    qDebug() << "hubcallback received";
    delete c; //VERY IMPORTANT, otherwise the callback will not be deleted -> memory leak
}

void Client::timerTick()
{
    qApp->exit(3);
}

