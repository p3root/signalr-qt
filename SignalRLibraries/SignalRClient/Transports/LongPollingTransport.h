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

#ifndef LONGPOLLINGTRANSPORT_H
#define LONGPOLLINGTRANSPORT_H

#include "HttpBasedTransport.h"
#include "Connection.h"
#include "SignalException.h"
#include <QThread>
#include <QTimer>


namespace P3 { namespace SignalR { namespace Client {

class SIGNALR_EXPORT LongPollingTransport : public HttpBasedTransport
{
    Q_OBJECT
public:
    LongPollingTransport();
    ~LongPollingTransport(void);

    void start(QString connection) OVERRIDE_M;

    void retry() OVERRIDE_M;

    const QString& getTransportType() OVERRIDE_M;

private:
    void startConnection();

private Q_SLOTS:
    void onPollHttpResponse(const QString& httpResponse, QSharedPointer<SignalException> error);
    void onConnectRequestFinished(const QString& httpResponse, QSharedPointer<SignalException> error);

    void keepAliveTimerTimeout();
    void errorRetryTimer();
    void reconnectErrorRetry();

    void reconnectErrorRetryWithStateChanging();
    void errorRetryWithStateChanging();

    void errorStartRetry();

private:
    bool _started;
    QString _url;
    QTimer _keepAliveTimer;
    QSharedPointer<SignalException> _lastSignalException;
    QTimer _retryTimerTimeout;
    bool _getOpen;

    QDateTime _lastMessageDate;
    QString _lastMessage;
    int _sameMessageCount;

};

}}}

#endif
