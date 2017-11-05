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

#ifndef HUBPROXY_H
#define HUBPROXY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include "HubCallback.h"

namespace P3 { namespace SignalR { namespace Client {

class HubConnection;

class SIGNALR_EXPORT HubProxy : public QObject
{
    Q_OBJECT

public:
    HubProxy(HubConnection* connection, QString hubName, QObject *objectToInvoke=0, Qt::ConnectionType conType =Qt::DirectConnection);
    ~HubProxy();

    void invoke(const QString &method, const QString &param, HubCallback* callback = 0);
    void invoke(const QString &method, const QStringList &params, HubCallback* callback = 0);
    void invoke(const QString &method, const QVariant &param, HubCallback* callback = 0);
    void invoke(const QString &method, const QVariantList &params, HubCallback* callback = 0);
    void invoke(const QString &method, HubCallback* callback = 0);

    QVariant syncInvoke(const QString &method, const QString &param, int timeoutMs = 10000, bool *ok=0);
    QVariant syncInvoke(const QString &method, const QStringList &param, int timeoutMs = 10000, bool *ok=0);

    QVariant syncInvoke(const QString &method, const QVariant& param, int timeoutMs = 10000, bool *ok=0);
    QVariant syncInvoke(const QString &method, const QVariantList &param, int timeoutMs = 10000, bool *ok=0);


    void onReceive(const QVariant &var);

    const QString& getName() const { return _hubName; }

    void addObjectToInvoke(QObject *obj);
    void removeObjectToInvoke(QObject *obj);

protected:
    virtual void send(const QString &data, const QString &id, HubCallback *c);

Q_SIGNALS:
    void hubMethodCalled(const QString &method, const QVariantList &args);

private:
    QGenericArgument getGenericArgument(const QString &type, const QString& val);

private:
    QList<QObject*> _objectsToInvoke;
    HubConnection* _connection;
    const QString _hubName;
    Qt::ConnectionType _connectionType;

    friend class HubConnection;
};

}}}

#endif // HUBPROXY_H
