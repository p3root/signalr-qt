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

#ifndef HUBPROXY_H
#define HUBPROXY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include "HubCallback.h"


namespace P3 { namespace SignalR { namespace Client {

class HubConnection;

class HubProxy : public QObject
{
    Q_OBJECT

public:
    HubProxy(HubConnection* connection, QString hubName, QObject *objectToInvoke=0);
    ~HubProxy();

    void invoke(QString method, QString param, HubCallback* callback = 0);
    void invoke(QString method, QStringList params, HubCallback* callback = 0);
    void invoke(QString method, QVariant param, HubCallback* callback = 0);
    void invoke(QString method, QVariantList params, HubCallback* callback = 0);
    void invoke(QString method, HubCallback* callback = 0);

    QVariant syncInvoke(QString method, QString param, int timeoutMs = ULONG_MAX, bool *ok=0);
    QVariant syncInvoke(QString method, QStringList param, int timeoutMs = ULONG_MAX, bool *ok=0);

    void onReceive(QVariant var);

    const QString& getName();

Q_SIGNALS:
    void hubMethodCalled(const QVariant &method, const QVariantList &args);
    void hubMessageReceived(const QVariant &data);

private:
    QGenericArgument getGenericArgument(const QString &type, const QString &val);

private:
    HubConnection* _connection;
    QObject *_objectToInvoke;
    const QString _hubName;
};

}}}

#endif // HUBPROXY_H
