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

#include "HubProxy.h"
#include "HubConnection.h"
#include <QextJson.h>
#include "Connection_p.h"

namespace P3 { namespace SignalR { namespace Client {

HubProxy::HubProxy(HubConnection* connection, QString hubName, QObject *objectToInvoke) : _connection(connection), _hubName(hubName)
{
    _objectToInvoke = objectToInvoke;
}

HubProxy::~HubProxy()
{
}

void HubProxy::invoke(const QString &method, const QString &param, HubCallback *callback)
{
    invoke(method, QStringList() << param, callback);
}

void HubProxy::invoke(const QString &method, const QStringList &params, HubCallback *callback)
{
    QVariantList variant;

    foreach(QString str, params)
    {
        variant << str;
    }

    invoke(method, variant, callback);
}

void HubProxy::invoke(const QString &method, HubCallback *callback)
{
    invoke(method, QVariantList(), callback);
}

QVariant HubProxy::syncInvoke(const QString &method, const QString &param, int timeoutMs, bool *ok)
{
    QVariant par(param);

    return syncInvoke(method, par, timeoutMs, ok);
}

QVariant HubProxy::syncInvoke(const QString &method, const QStringList &params, int timeoutMs, bool *ok)
{
    QVariantList variant;

    foreach(QString str, params)
    {
        variant << str;
    }

    return syncInvoke(method, variant, timeoutMs, ok);
}

QVariant HubProxy::syncInvoke(const QString &method, const QVariant &param, int timeoutMs, bool *ok)
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

QVariant HubProxy::syncInvoke(const QString &method, const QVariantList &param, int timeoutMs, bool *ok)
{
    QEventLoop loop;

    if(timeoutMs < 0)
        timeoutMs = 100000;

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

void HubProxy::invoke(const QString &method, const QVariant &param, HubCallback* callback)
{
    invoke(method, QVariantList() << param, callback);
}

void HubProxy::invoke(const QString &method, const QVariantList &params, HubCallback* callback)
{
    QString i = QString::number(_connection->getNextCount());
    QVariantMap map;
    map.insert("A", params);
    map.insert("I", i);
    map.insert("H", _hubName);
    map.insert("M", method);
    send(QextJson::stringify(QVariant::fromValue(map)), i, callback);
}

void HubProxy::onReceive(const QVariant &var)
{
    QVariantMap qvl = var.toMap();

    if(_objectToInvoke && qvl.contains(("M")) && qvl.contains("A"))
    {
        QString method = qvl["M"].toString();
        method[0] = method[0].toLower();
        QVariantList args = qvl["A"].toList();
        const QMetaObject *meta = _objectToInvoke->metaObject();
        bool invokeOk = false;

        for(int i = meta->methodOffset(); i < meta->methodCount(); ++i)
        {
            QString curMethod = QString::fromLatin1(meta->method(i).signature());

            if(curMethod.startsWith(method))
            {
                curMethod.remove(0, method.length()); //remove method name
                curMethod.remove(0, 1); //remove open brace
                curMethod.remove(curMethod.count()-1, 1); //remove close brace

                QStringList params = curMethod.split(','); //split params

                QStringList tmpParams;
                foreach(QString param, params)
                {
                    tmpParams << param.replace(";", ",");
                }
                params = tmpParams;

                if(params.count() != args.count())
                {
                    _connection->getConnectionPrivate()->emitLogMessage("Invalid size in give and needed args found, hubMethodCalled will be emited", SignalR::Debug);
                    Q_EMIT hubMethodCalled(method, args);
                    return;
                }

                //i have no fucking idea why I cannot pass template class as args in QMetaObject::invoke
                //so i just stringify it an pass it to the method as a string
                int x = 0;
                foreach(QVariant arg, args)
                {
                    if(arg.type() == QVariant::Map
                            || arg.type() == QVariant::List)
                    {
                        args[x] = QextJson::stringify(arg);
                    }
                    x++;
                }

                bool retVal = true;

                switch(params.length())
                {
                case 1:
                    retVal = QMetaObject::invokeMethod(_objectToInvoke, method.toStdString().c_str(),
                                                       getGenericArgument(params[0], args[0].toString()));
                    break;
                case 2:
                    retVal = QMetaObject::invokeMethod(_objectToInvoke, method.toStdString().c_str(),
                                                       getGenericArgument(params[0], args[0].toString()),
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
                    _connection->getConnectionPrivate()->emitLogMessage("Not more then 10 params allow in dynamic invokation", SignalR::Debug);
                    Q_EMIT hubMethodCalled(method, args);
                    break;
                }

                if(retVal)
                {
                    invokeOk = true;
                }
            }
        }
        if(!invokeOk)
        {
            _connection->getConnectionPrivate()->emitLogMessage("Could not dynamically invoke method", SignalR::Debug);
            Q_EMIT hubMethodCalled(method, args);
        }
    }
    else
    {
        QVariantMap qvl = var.toMap();
        if(qvl.contains(("M")) && qvl.contains("A"))
        {
            QString method = qvl["M"].toString();
            QVariantList args = qvl["A"].toList();
            Q_EMIT hubMethodCalled(method, args);
        }
        else
        {
            _connection->getConnectionPrivate()->emitLogMessage("HubProxy Message with no Method name called", SignalR::Warning);
        }
    }
}

void HubProxy::send(const QString &data, const QString &id, HubCallback *c)
{
    _connection->send(data, id, c);
}

QGenericArgument HubProxy::getGenericArgument(const QString &type, const QString &val)
{
    if(type == "QString")
    {
        return Q_ARG(QString, val);
    }
    else if(type == "int")
    {
        return Q_ARG(int, int(val.toInt()));
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


    _connection->getConnectionPrivate()->emitLogMessage("getGenericArgument: no type found for " + type + " it will be seen as a QString", SignalR::Warning);
    return Q_ARG(QString, val);
}

}}}
