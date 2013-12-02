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

#include "HubConnection.h"
#include <QextJson.h>

HubConnection::HubConnection(QString url) : Connection(url)
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

void HubConnection::send(QString data, QString id, HubCallback *c)
{
    _callbacks.insert(id, c);
    Connection::send(data);
}

void HubConnection::stop()
{
    Connection::stop();
}

HubProxy *HubConnection::createHubProxy(QString name)
{
    if(_state != Disconnected)
    {
        if(getLogErrorsToQDebug())
        {
            qCritical() << "A HubProxy cannot be added after the connection has been started.";
        }

        emitLogMessage("A HubProxy cannot be added after the connection has been started", Connection::Error);
        return 0;
    }

    HubProxy* proxy;
    if(!_hubs.contains(name))
    {
        proxy = new HubProxy(this, name);
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

void HubConnection::onReceived(QVariant data)
{
    if(data.convert(QVariant::Map))
    {
        QVariantMap map = data.value<QVariantMap>();

        //if message is from hub, search hub and write message to it
        if(map.contains("H"))
        {
            QVariant hub = map["H"];

            if(_hubs.contains(hub.toString()))
            {
                _hubs[hub.toString()]->onReceive(data);
            }
        }
        else if(map.contains("I"))
        {
            QVariant id = map["I"];

            if(_callbacks.contains(id.toString()))
            {
                HubCallback* callback = _callbacks[id.toString()];
                if(callback)
                    Q_EMIT callback->raiseMessageReceived(data);
                _callbacks.remove(id.toString());
            }
        }
        else
        {
            Connection::onReceived(data);
        }
    }
}

HubProxy *HubConnection::getByName(const QString &name)
{
    if(!_hubs.contains(name))
    {
        if(getLogErrorsToQDebug())
            qCritical() << "could not find proxy with name " << name;

        emitLogMessage("Cloud not find proxy with name " + name, Connection::Error);
    }
    return _hubs[name];
}
