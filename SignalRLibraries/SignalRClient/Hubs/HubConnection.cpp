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

#include "HubConnection.h"
#include <QextJson.h>

#include "Connection_p.h"

namespace P3 { namespace SignalR { namespace Client {

HubConnection::HubConnection(const QString &url) : Connection(url), _callbacksMutex(QMutex::Recursive)
{

}

HubConnection::~HubConnection()
{
    for(int i = 0; i < _hubs.count(); i++)
    {
        HubProxy* hub = _hubs.values().at(i);;
        _hubs.remove(_hubs.keys()[i]);
        delete hub;
    }
}

void HubConnection::send(const QString &data, const QString &id, HubCallback *c)
{
    {
        QMutexLocker l(&_callbacksMutex);

        if(_callbacks.contains(id))
             d_ptr->emitLogMessage("Duplicated HUB ID!", SignalR::Error);

        if (c)
            _callbacks.insert(id, c);
    }

    Connection::send(data);
}

bool HubConnection::stop(int timeoutMs)
{
    return Connection::stop(timeoutMs);
}

HubProxy *HubConnection::createHubProxy(QString name, QObject *objectToInvoke, Qt::ConnectionType conType)
{
    if(_hubs.contains(name))
    {
        _hubs[name]->addObjectToInvoke(objectToInvoke);
        _hubs[name]->_connectionType = conType;
        return _hubs[name];
    }
    if(getState() != SignalR::Disconnected)
    {
        QString objToInvoke = QString(objectToInvoke->metaObject()->className());
        d_ptr->emitLogMessage(QString("A HubProxy (%1 - %2) cannot be added after the connection has been started").arg(name, objToInvoke), SignalR::Error);
        return 0;
    }

    HubProxy* proxy;
    if(!_hubs.contains(name))
    {
        proxy = newHubProxy(name, objectToInvoke, conType);
        _hubs.insert(name, proxy);
        return proxy;
    }
    return _hubs[name];
}

QString HubConnection::onSending()
{
    QVariantList lst;
    QVariantMap map;
    for(int i = 0; i < _hubs.count(); i++)
    {
        map.insert("Name", _hubs.keys().at(i));
        lst.append(map);
    }
    

    QString json = QextJson::stringify(QVariant::fromValue(lst));
    return json;
}

void HubConnection::onReceived(QVariant &data)
{
    if(data.convert(QVariant::Map))
    {
        QVariantMap map = data.value<QVariantMap>();

        if(map.contains("E")) {
            Q_EMIT exceptionReceived(map["E"].toString(), map["T"].toString());
        }
        else if(map.contains("H")) //if message is from hub, search hub and write message to it
        {
            QVariant hub = map["H"];

            if(_hubs.contains(hub.toString()))
            {
                _hubs[hub.toString()]->onReceive(data);
            }
        }
        else if(map.contains("I"))
        {
            QMutexLocker l(&_callbacksMutex);
            QString id = map["I"].toString();
            HubCallback* callback = _callbacks.value(id);
            if(callback)
            {
                _callbacks.remove(id);
                Q_EMIT callback->raiseMessageReceived(data);
            }
        }
        else
        {
            Connection::onReceived(data);
        }
    }
    else
    {
        Connection::onReceived(data);
    }
}

HubProxy *HubConnection::getByName(const QString &name)
{
    if(!_hubs.contains(name))
    {
        d_ptr->emitLogMessage("Cloud not find proxy with name " + name, SignalR::Error);
        return 0;
    }
    return _hubs[name];
}

quint64 HubConnection::getNextCount()
{
    return d_ptr->getNextCount();
}

HubProxy *HubConnection::newHubProxy(const QString &name, QObject *objectToInvoke, Qt::ConnectionType conType)
{
    return new HubProxy(this, name, objectToInvoke, conType);
}

}}}
