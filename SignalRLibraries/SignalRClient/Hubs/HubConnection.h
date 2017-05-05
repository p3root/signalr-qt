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

#ifndef HUBCONNECTION_H
#define HUBCONNECTION_H

#include "Connection.h"
#include <QtCore>
#include "HubProxy.h"
#include <QStringList>
#include "HubCallback.h"
#include <QObject>


namespace P3 { namespace SignalR { namespace Client {

class SIGNALR_EXPORT HubConnection : public Connection
{
public:
    HubConnection(const QString &url);
    ~HubConnection();

    virtual void send(const QString &data, const QString &id, HubCallback*);
    virtual bool stop(int timeoutMs=0);

    HubProxy* createHubProxy(QString name, QObject *objectToInvoke=0, Qt::ConnectionType conType = Qt::AutoConnection);

    QString onSending();
    virtual void onReceived(QVariant &data);

    HubProxy* getByName(const QString& name);

    quint64 getNextCount();

friend class HubProxy;

protected:
    virtual HubProxy* newHubProxy(const QString &name, QObject *objectToInvoke=0, Qt::ConnectionType conType = Qt::AutoConnection);

protected:
    QMap<QString, HubProxy*> _hubs;

private:
    QMap<QString, HubCallback*> _callbacks;
    QMutex _callbacksMutex;

};

}}}

#endif // HUBCONNECTION_H
