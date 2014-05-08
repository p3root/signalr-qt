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

#ifndef HEARTBEATMONITOR_H
#define HEARTBEATMONITOR_H

#include <QMutex>
#include <QTimer>
#include <SignalR.h>
#include "SignalR_global.h"


namespace P3 { namespace SignalR { namespace Client {

class ConnectionPrivate;

class SIGNALR_EXPORT HeartbeatMonitor : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(HeartbeatMonitor)
public:
    explicit HeartbeatMonitor(ConnectionPrivate *con, QMutex* stateLocker=0);

    void start();
    void stop();
    void beat(double timeElapsed);

Q_SIGNALS:
    void stopCrossThread();


private Q_SLOTS:
    void beat();
    void onStop();

private:
    bool checkKeepAliveData();

private:
    ConnectionPrivate *_connection;
    QMutex *_locker;

    QTimer _timer;
    bool _hasBeenWarned;
    bool _timedOut;
};

}}}

#endif // HEARTBEATMONITOR_H
