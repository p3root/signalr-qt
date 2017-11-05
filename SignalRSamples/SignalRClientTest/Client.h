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

#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTimer>

#include <Hubs/HubConnection.h>
#include <Transports/HttpClient.h>
#include <Transports/LongPollingTransport.h>
#include <Helper/Helper.h>
#include <Transports/WebSocketTransport.h>
#include <Transports/ServerSentEventsTransport.h>
#include <Transports/AutoTransport.h>
#include <HeartbeatMonitor.h>

using namespace P3::SignalR::Client;

class Client : public QObject
{
    Q_OBJECT
public:
    Client(QCoreApplication &app);
    ~Client();

    void start();

private Q_SLOTS:
    void stop();
    void timerTick();

    void onMethodCalled(const QString& method, const QVariantList &args);
    void onError(QSharedPointer<SignalException>);
    void onStateChanged(SignalR::State oldState, SignalR::State newState);
    void answerReceived(HubCallback*, QVariant);
    void onLogMessage(QString, int severity);

    void send(QString message);
    void sendFloat(float number);

private:
    static int test(HubConnection* t);

    QString getStateAsText(SignalR::State);

private:
    QTimer _timer;
    HubConnection* _connection;
    HttpClient* _client;
    ClientTransport* _transport;
    HeartbeatMonitor *_monitor;
};

#endif // CLIENT_H
