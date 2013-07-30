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

#include "HeartbeatMonitor.h"
#include <QDebug>

HeartbeatMonitor::HeartbeatMonitor(Connection *con, QMutex *stateLocker)
{
    _connection = con;
    _locker = stateLocker;
    _timedOut = false;
    _hasBeenWarned = false;

}

void HeartbeatMonitor::start()
{
    if(!checkKeepAliveData())
        return;

    connect(&_timer, SIGNAL(timeout()), this, SLOT(beat()));
    int tick = _connection->getKeepAliveData().getCheckInterval() * 1000;
    _timer.setInterval(tick);
    _timer.start();
}

void HeartbeatMonitor::stop()
{
    _timer.stop();
}

void HeartbeatMonitor::beat(double timeElapsed)
{
    if(!checkKeepAliveData())
        return;

    if(_locker)
        QMutexLocker l(_locker);

    if(_connection->getState() == Connection::Connected)
    {
        if(timeElapsed >= _connection->getKeepAliveData().getTimeout())
        {
            if(!_timedOut)
            {
                qCritical() << "Connection Timed-Out";
                _timedOut = true;
                _connection->getTransport()->lostConnection(_connection);
            }
        }
        else if(timeElapsed >= _connection->getKeepAliveData().getTimeoutWarning())
        {
            if(!_hasBeenWarned)
            {
                qWarning() << "Connection Timeout-Warning";
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

    double timeElapsed = QDateTime::currentDateTime().secsTo(_connection->getKeepAliveData().getLastKeepAlive()) * -1;
    beat(timeElapsed);
}

bool HeartbeatMonitor::checkKeepAliveData()
{
    if(!&_connection->getKeepAliveData())
    {
        return false;
    }

    return true;
}
