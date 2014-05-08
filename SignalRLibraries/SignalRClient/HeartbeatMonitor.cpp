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

#include "HeartbeatMonitor.h"
#include <QDebug>
#include "Connection_p.h"

namespace P3 { namespace SignalR { namespace Client {

HeartbeatMonitor::HeartbeatMonitor(ConnectionPrivate *con, QMutex *stateLocker)
{
    _connection = con;
    _locker = stateLocker;
    _timedOut = false;
    _hasBeenWarned = false;
    connect(this, SIGNAL(stopCrossThread()), this, SLOT(onStop()));
}

void HeartbeatMonitor::start()
{
    if(!checkKeepAliveData())
        return;

    connect(&_timer, SIGNAL(timeout()), this, SLOT(beat()));
    int tick = _connection->getKeepAliveData()->getCheckInterval() * 1000;
    _timer.setInterval(tick);
    _timer.start();
}

void HeartbeatMonitor::stop()
{
    Q_EMIT stopCrossThread();;
}

void HeartbeatMonitor::beat(double timeElapsed)
{
    if(!checkKeepAliveData())
        return;

    QMutexLocker l(_locker);

    if(_connection->getState() == SignalR::Connected)
    {
        if(timeElapsed >= _connection->getKeepAliveData()->getTimeout())
        {
            if(!_timedOut)
            {
                _connection->emitLogMessage("Connection Timeout-Out", SignalR::Error);
                _timedOut = true;
                _connection->getTransport()->lostConnection(_connection);
            }
        }
        else if(timeElapsed >= _connection->getKeepAliveData()->getTimeoutWarning())
        {
            if(!_hasBeenWarned)
            {
                _connection->emitLogMessage("Connection Timeout-Warning", SignalR::Warning);
                _hasBeenWarned = true;
                _connection->connectionSlow();
            }
        }
        else
        {
            _hasBeenWarned = false;
            _timedOut = false;
        }
    }
}

void HeartbeatMonitor::beat()
{
    if(!checkKeepAliveData())
        return;

    double timeElapsed = QDateTime::currentDateTimeUtc().secsTo(_connection->getKeepAliveData()->getLastKeepAlive()) * -1;
    beat(timeElapsed);
}

void HeartbeatMonitor::onStop()
{
    _timer.stop();
}

bool HeartbeatMonitor::checkKeepAliveData()
{
    if(!_connection->getKeepAliveData())
    {
        return false;
    }

    return true;
}

}}}
