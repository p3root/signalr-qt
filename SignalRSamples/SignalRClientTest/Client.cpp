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

#include "Client.h"

#include <QCoreApplication>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
    #include <QtConcurrent/QtConcurrent>
#endif

Client::Client(QCoreApplication &app)
{
    //_timer.setSingleShot(true);
    _timer.setInterval(5000);
    _timer.start();
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
    _connection = new HubConnection("http://192.168.8.119:8080/signalr");
    _connection->setIgnoreSslErrors(true);
    _connection->setReconnectWaitTime(3);
    _monitor = &_connection->getHeartbeatMonitor();

    _transport = new AutoTransport();

    HubProxy* proxy = _connection->createHubProxy("Chat", this);

    connect(proxy, SIGNAL(hubMethodCalled(QString,QVariantList)), this, SLOT(onMethodCalled(QString,QVariantList)));

    connect(_connection, SIGNAL(errorOccured(QSharedPointer<SignalException>)), this, SLOT(onError(QSharedPointer<SignalException>)));
    connect(_connection, SIGNAL(stateChanged(SignalR::State,SignalR::State)), this, SLOT(onStateChanged(SignalR::State,SignalR::State)));
    connect(_connection, SIGNAL(logMessage(QString,int)), this, SLOT(onLogMessage(QString,int)));

    QList<QPair<QString, QString> > headers;
    QPair<QString, QString> data("1", "2");
    headers.append(data);

    _connection->setAdditionalHttpHeaders(headers);
    _connection->setAdditionalQueryString(headers);

    _connection->start(_transport, true);


}

void Client::stop()
{
    if(_connection->stop(5000))
    {
        qDebug() << "successfully disconnected from signalr server";
    }
    else
    {
        qWarning() << "could not disconnect from signalr server";
    }
}

void Client::onMethodCalled(const QString &method, const QVariantList &args)
{
    qDebug() << method << " " << args;
}

void Client::onError(QSharedPointer<SignalException> error)
{
     qDebug() << error->what();
}

void Client::onStateChanged(SignalR::State oldState, SignalR::State newState)
{
    qDebug()  << "state changed: " << getStateAsText(oldState) << " -> " << getStateAsText(newState) << " << " << _connection->getConnectionId();

    if(newState == SignalR::Connected)
    {
        HubCallback* callback = new HubCallback(0);
        connect(callback, SIGNAL(messageReceived(HubCallback*,QVariant)), this, SLOT(answerReceived(HubCallback*,QVariant)));
        HubProxy* prox = _connection->getByName("Chat");
        prox->invoke("Send", QString("message"), callback);

       // QFuture<int> res = QtConcurrent::run(Client::test, _connection);
        //res.waitForFinished();
    }
    else if(newState == SignalR::Disconnected)
    {
        disconnect(this, SLOT(answerReceived(HubCallback*,QVariant)));
    }
    else if(newState == SignalR::Reconnecting)
    {
        QFuture<int> res = QtConcurrent::run(Client::test, _connection);
        res.waitForFinished();
    }
}

void Client::answerReceived(HubCallback *c, QVariant v)
{
    Q_UNUSED(v);
    qDebug() << "hubcallback received";
    delete c; //VERY IMPORTANT, otherwise the callback will not be deleted -> memory leak
}

void Client::onLogMessage(QString msg, int severity)
{
    Q_UNUSED(severity);
    qDebug() << msg;
}

void Client::send(QString message)
{
    qDebug() << "send method called : " << message;
}

void Client::sendFloat(float number)
{
    qDebug() << number;
}

int Client::test(HubConnection *t)
{
    //HubProxy* prox = t->getByName("Chat");
   // prox->invoke("Send", QString("message"));

    t->retry();
    return 0;
}

QString Client::getStateAsText(SignalR::State state)
{

    if(state == SignalR::Connected)
        return "Connected";
    else if(state == SignalR::Connecting)
        return "Connecting";
    else if(state == SignalR::Disconnected)
        return "Disconnected";
    else if(state == SignalR::Disconnecting)
        return "Disconnecting";
    else if(state == SignalR::Reconnecting)
        return "Reconnecting";
    return "UNKOWN";

}

void Client::timerTick()
{
    static int x = 0;
    auto msg = QString("message %1").arg(x);
    qDebug() << "send message to server "<<msg;
    HubProxy* prox = _connection->getByName("Chat");
    prox->invoke("SendFloat", msg, 0);
    x++;

}

