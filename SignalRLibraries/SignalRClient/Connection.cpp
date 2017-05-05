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

#include "Connection.h"
#include "Connection_p.h"

#include <QString>
#include "Transports/HttpClient.h"
#include "Helper/Helper.h"
#include "Transports/AutoTransport.h"

namespace P3 { namespace SignalR { namespace Client {

Connection::Connection(const QString &host) :
    d_ptr(new ConnectionPrivate(host, this, this))
{


}

Connection::~Connection()
{

}

void Connection::start(bool autoReconnect)
{
    Q_D(Connection);
    d->start(autoReconnect);
}

void Connection::start(ClientTransport* transport, bool autoReconnect)
{	
    Q_D(Connection);
    d->start(transport, autoReconnect);
}

void Connection::send(const QString &data)
{
    Q_D(Connection);
    d->send(data);
}

void Connection::retry()
{
    Q_D(Connection);
    d->retry();
}

SignalR::State Connection::getState()
{
    Q_D(Connection);
    return d->getState();
}

const QString &Connection::getConnectionId() const
{
    const Q_D(Connection);
    return d->getConnectionId();
}


QString Connection::onSending()
{
    Q_D(Connection);
    return d->onSending();
}

const QList<QPair<QString, QString> > &Connection::getAdditionalHttpHeaders()
{
    Q_D(Connection);
    return d->getAdditionalHttpHeaders();
}

void Connection::setAdditionalHttpHeaders(QList<QPair<QString, QString> > lst)
{
    Q_D(Connection);
    d->setAdditionalHttpHeaders(lst);
}

const QList<QPair<QString, QString> > &Connection::getAdditionalQueryString()
{
    Q_D(Connection);
    return d->getAdditionalQueryString();
}

void Connection::setAdditionalQueryString(QList<QPair<QString, QString> > lst)
{
    Q_D(Connection);
    d->setAdditionalQueryString(lst);
}

const KeepAliveData *Connection::getKeepAliveData()
{
    Q_D(Connection);
    return d->getKeepAliveData();
}


void Connection::onReceived(QVariant &data)
{
    Q_UNUSED(data);
}

ClientTransport *Connection::getTransport()
{
    Q_D(Connection);
    return d->getTransport();
}

const QString &Connection::getUrl() const
{
    const Q_D(Connection);
    return d->getUrl();
}

const QString &Connection::getConnectionToken() const
{
    const Q_D(Connection);
    return d->getConnectionToken();
}

const QString &Connection::getGroupsToken() const
{
    const Q_D(Connection);
    return d->getGroupsToken();
}

void Connection::presetCount(quint64 preset)
{
    Q_D(Connection);
    d->presetCount(preset);
}

bool Connection::getAutoReconnect() const
{
    const Q_D(Connection);
    return d->getAutoReconnect();
}

bool Connection::stop(int timeoutMs)
{
    Q_D(Connection);
    return d->stop(timeoutMs);
}

void Connection::setProxySettings(const QNetworkProxy proxy)
{
    Q_D(Connection);
    d->setProxySettings(proxy);
}

const QNetworkProxy &Connection::getProxySettings()
{
    Q_D(Connection);
    return d->getProxySettings();
}

int Connection::getReconnectWaitTime()
{
    Q_D(Connection);
    return d->getReconnectWaitTime();
}

void Connection::setReconnectWaitTime(int timeInMilliSeconds)
{
    Q_D(Connection);
    d->setReconnectWaitTime(timeInMilliSeconds);
}

const QString &Connection::getProtocolVersion()
{
    Q_D(Connection);
    return d->getProtocolVersion();
}

bool Connection::ignoreSslErrors()
{
    Q_D(Connection);
    return d->ignoreSslErrors();
}

void Connection::setIgnoreSslErrors(bool ignoreSslErrors)
{
    Q_D(Connection);
    d->setIgnoreSslErrors(ignoreSslErrors);
}

void Connection::setSslConfiguration(const QSslConfiguration &config)
{
    Q_D(Connection);
    d->setSslConfiguration(config);
}

const QSslConfiguration &Connection::getSslConfiguration()
{
    Q_D(Connection);
    return d->getSslConfiguration();
}

HeartbeatMonitor &Connection::getHeartbeatMonitor()
{
    Q_D(Connection);
    return d->getHeartbeatMonitor();
}

int Connection::getPostTimeoutMs()
{
    Q_D(Connection);
    return d->getPostTimeoutMs();
}

void Connection::setPostTimeOutMs(int timeoutMs)
{
    Q_D(Connection);
    d->setPostTimeOutMs(timeoutMs);
}

const QString &Connection::getTid()
{
    Q_D(Connection);
    return d->getTid();
}

void Connection::setTid(const QString &tid)
{
    Q_D(Connection);
    d->setTid(tid);
}

void Connection::setMessageRepeatReconInterval(int intervalMs)
{
    Q_D(Connection);
    d->setMessageRepeatReconInterval(intervalMs);
}

int Connection::messageRepeatReconInterval()
{
    Q_D(Connection);
    return d->messageRepeatReconInterval();
}

void Connection::setMessageRepeatReconAmount(int amount)
{
    Q_D(Connection);
    d->setMessageRepeatReconAmount(amount);
}

int Connection::messageRepeatReconAmount()
{
    Q_D(Connection);
    return d->messageRepeatReconAmount();
}

void Connection::onTransportStarted(QSharedPointer<SignalException>)
{

}

void Connection::onMessageSentCompleted(QSharedPointer<SignalException>, quint64)
{

}

void Connection::transportStarted(QSharedPointer<SignalException> error)
{
    Q_D(Connection);
    d->transportStarted(error);
}

void Connection::transportMessageSent(QSharedPointer<SignalException> ex, quint64 messageId)
{
    Q_D(Connection);
    d->transportMessageSent(ex, messageId);
}

}}}
