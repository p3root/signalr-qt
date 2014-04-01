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

#include "HubProxy.h"
#include "HubConnection.h"
#include <QextJson.h>

namespace P3 { namespace SignalR { namespace Client {

HubProxy::HubProxy(HubConnection* connection, QString hubName, QObject *objectToInvoke) : _connection(connection), _hubName(hubName)
{
    _objectToInvoke = objectToInvoke;
}

HubProxy::~HubProxy()
{
}

void HubProxy::invoke(QString method, QString param, HubCallback *callback)
{
    invoke(method, QStringList() << param, callback);
}

void HubProxy::invoke(QString method, QStringList params, HubCallback *callback)
{
    QVariantList variant;

    foreach(QString str, params)
    {
        variant << str;
    }

    invoke(method, variant, callback);
}

void HubProxy::invoke(QString method, HubCallback *callback)
{
    invoke(method, QVariantList(), callback);
}

QVariant HubProxy::syncInvoke(QString method, QString param, int timeoutMs, bool *ok)
{
    QEventLoop loop;

    HubCallback *callback = new HubCallback(0, method);
    QTimer timer;
    timer.setInterval(timeoutMs);
    timer.setSingleShot(true);
    timer.start();
    invoke(method, param, callback);

    while(true)
    {
        loop.processEvents(QEventLoop::AllEvents, 100);

        if(callback->isFinished() || !timer.isActive())
            break;
    }
    if(ok)
    {
        if(!timer.isActive())
        {
            *ok = false;
        }
        else
        {
            *ok = true;
        }
    }
    QVariant data = callback->data();
    delete callback;
    return data;
}

QVariant HubProxy::syncInvoke(QString method, QStringList param, int timeoutMs, bool *ok)
{
    QEventLoop loop;

    HubCallback *callback = new HubCallback(0, method);
    QTimer timer;
    timer.setInterval(timeoutMs);
    timer.setSingleShot(true);
    timer.start();
    invoke(method, param, callback);

    while(true)
    {
        loop.processEvents(QEventLoop::AllEvents, 100);

        if(callback->isFinished() || !timer.isActive())
            break;
    }
    if(ok)
    {
        if(!timer.isActive())
        {
            *ok = false;
        }
        else
        {
            *ok = true;
        }
    }
    QVariant data = callback->data();
    delete callback;
    return data;
}

void HubProxy::invoke(QString method, QVariant param, HubCallback* callback)
{
    invoke(method, QVariantList() << param, callback);
}

void HubProxy::invoke(QString method, QVariantList params, HubCallback* callback)
{
    QVariantMap map;
    map.insert("A", params);
    map.insert("I", QString::number(_connection->getCount()));
    map.insert("H", _hubName);
    map.insert("M", method);
    _connection->send(QextJson::stringify(QVariant::fromValue(map)), QString::number(_connection->getCount()), callback);
}

void HubProxy::onReceive(QVariant var)
{
    QVariantMap qvl = var.toMap();

    if(_objectToInvoke && qvl.contains(("M")) && qvl.contains("A"))
    {
        QString method = qvl["M"].toString();
        QVariantList args = qvl["A"].toList();
        const QMetaObject *meta = _objectToInvoke->metaObject();

        for(int i = meta->methodOffset(); i < meta->methodCount(); ++i)
        {
            QString curMethod = QString::fromLatin1(meta->method(i).signature());

            if(curMethod.startsWith(method))
            {
                curMethod.remove(method); //remove method name
                curMethod.remove(0, 1); //remove open brace
                curMethod.remove(curMethod.count()-1, 1); //remove close brace

                QStringList params = curMethod.split(','); //split params

                if(params.count() != args.count())
                {
                    qDebug() << "invalid size in give and needed args found, hubMethodCalled will be emited";
                    Q_EMIT hubMethodCalled(method, args);
                    return;
                }
                bool retVal = true;

                switch(params.length())
                {
                case 1:
                    retVal = QMetaObject::invokeMethod(_objectToInvoke, method.toStdString().c_str(), getGenericArgument(params[0], args[0].toString()));
                    break;
                case 2:
                    retVal = QMetaObject::invokeMethod(_objectToInvoke, method.toStdString().c_str(), getGenericArgument(params[0], args[0].toString()),
                                                                                                      getGenericArgument(params[1], args[1].toString()));
                    break;
                case 3:
                    retVal = QMetaObject::invokeMethod(_objectToInvoke, method.toStdString().c_str(),
                                                        getGenericArgument(params[0], args[0].toString()),
                                                        getGenericArgument(params[1], args[1].toString()),
                                                        getGenericArgument(params[2], args[2].toString()));
                    break;
                case 4:
                    retVal = QMetaObject::invokeMethod(_objectToInvoke, method.toStdString().c_str(),
                                                        getGenericArgument(params[0], args[0].toString()),
                                                        getGenericArgument(params[1], args[1].toString()),
                                                        getGenericArgument(params[2], args[2].toString()),
                                                        getGenericArgument(params[3], args[3].toString()));

                    break;
                case 5:
                    retVal = QMetaObject::invokeMethod(_objectToInvoke, method.toStdString().c_str(),
                                                        getGenericArgument(params[0], args[0].toString()),
                                                        getGenericArgument(params[1], args[1].toString()),
                                                        getGenericArgument(params[2], args[2].toString()),
                                                        getGenericArgument(params[3], args[3].toString()),
                                                        getGenericArgument(params[4], args[4].toString()));

                    break;
                case 6:
                    retVal = QMetaObject::invokeMethod(_objectToInvoke, method.toStdString().c_str(),
                                                        getGenericArgument(params[0], args[0].toString()),
                                                        getGenericArgument(params[1], args[1].toString()),
                                                        getGenericArgument(params[2], args[2].toString()),
                                                        getGenericArgument(params[3], args[3].toString()),
                                                        getGenericArgument(params[4], args[4].toString()),
                                                        getGenericArgument(params[5], args[5].toString()));

                    break;
                case 7:
                    retVal = QMetaObject::invokeMethod(_objectToInvoke, method.toStdString().c_str(),
                                                        getGenericArgument(params[0], args[0].toString()),
                                                        getGenericArgument(params[1], args[1].toString()),
                                                        getGenericArgument(params[2], args[2].toString()),
                                                        getGenericArgument(params[3], args[3].toString()),
                                                        getGenericArgument(params[4], args[4].toString()),
                                                        getGenericArgument(params[5], args[5].toString()),
                                                        getGenericArgument(params[6], args[6].toString()));

                    break;
                case 8:
                    retVal = QMetaObject::invokeMethod(_objectToInvoke, method.toStdString().c_str(),
                                                          getGenericArgument(params[0], args[0].toString()),
                                                          getGenericArgument(params[1], args[1].toString()),
                                                          getGenericArgument(params[2], args[2].toString()),
                                                          getGenericArgument(params[3], args[3].toString()),
                                                          getGenericArgument(params[4], args[4].toString()),
                                                          getGenericArgument(params[5], args[5].toString()),
                                                          getGenericArgument(params[6], args[6].toString()),
                                                          getGenericArgument(params[7], args[7].toString()));

                    break;
                case 9:
                    retVal = QMetaObject::invokeMethod(_objectToInvoke, method.toStdString().c_str(),
                                                          getGenericArgument(params[0], args[0].toString()),
                                                          getGenericArgument(params[1], args[1].toString()),
                                                          getGenericArgument(params[2], args[2].toString()),
                                                          getGenericArgument(params[3], args[3].toString()),
                                                          getGenericArgument(params[4], args[4].toString()),
                                                          getGenericArgument(params[5], args[5].toString()),
                                                          getGenericArgument(params[6], args[6].toString()),
                                                          getGenericArgument(params[7], args[7].toString()),
                                                          getGenericArgument(params[8], args[8].toString()));
                    break;
                case 10:
                    retVal = QMetaObject::invokeMethod(_objectToInvoke, method.toStdString().c_str(),
                                                          getGenericArgument(params[0], args[0].toString()),
                                                          getGenericArgument(params[1], args[1].toString()),
                                                          getGenericArgument(params[2], args[2].toString()),
                                                          getGenericArgument(params[3], args[3].toString()),
                                                          getGenericArgument(params[4], args[4].toString()),
                                                          getGenericArgument(params[5], args[5].toString()),
                                                          getGenericArgument(params[6], args[6].toString()),
                                                          getGenericArgument(params[7], args[7].toString()),
                                                          getGenericArgument(params[8], args[8].toString()),
                                                          getGenericArgument(params[9], args[9].toString()));
                    break;
                default:
                    qDebug() << "not more then 10 params allowd in dynamic invoke, calling onHubMethodCalled";
                    Q_EMIT hubMethodCalled(method, args);
                    break;
                }

                if(!retVal)
                {
                    qDebug() << "it seems like something went wrong on invoking the method, calling onHubMethodCalled";
                     Q_EMIT hubMethodCalled(method, args);
                }


            }
        }
    }
    else
    {
        QVariantMap qvl = var.toMap();
        if(qvl.contains(("M")) && qvl.contains("A"))
        {
            QVariant method = qvl["M"];
            QVariantList args = qvl["A"].toList();
            Q_EMIT hubMethodCalled(method, args);
        }
        else
        {
            Q_EMIT hubMessageReceived(var);
        }
    }
}

const QString &HubProxy::getName()
{
    return _hubName;
}

QGenericArgument HubProxy::getGenericArgument(const QString &type, const QString &val)
{
    if(type == "QString")
    {
        return Q_ARG(QString, val);
    }
    else if(type == "int")
    {
        return Q_ARG(int, val.toInt());
    }
    else if(type == "float")
    {
        return Q_ARG(float, val.toFloat());
    }
    else if(type == "double")
    {
        return Q_ARG(double, val.toDouble());
    }
    else if(type == "short")
    {
        return Q_ARG(short, val.toShort());
    }
    else if(type == "ushort")
    {
        return Q_ARG(ushort, val.toUShort());
    }
    else if(type == "uint")
    {
        return Q_ARG(uint, val.toUInt());
    }
    else if(type == "long")
    {
        return Q_ARG(long, val.toLong());
    }
    else if(type == "ulong")
    {
        return Q_ARG(ulong, val.toULong());
    }
    else if(type == "qlonglong" || type == "long long" || type == "qint64")
    {
        return Q_ARG(qlonglong, val.toLongLong());
    }
    else if(type == "qulonglong" || type == "unsigned long long" || type == "quint64")
    {
        return Q_ARG(qulonglong, val.toULongLong());
    }

    qDebug() << "no type found for " << type;
    return Q_ARG(QString, val);
}

}}}
