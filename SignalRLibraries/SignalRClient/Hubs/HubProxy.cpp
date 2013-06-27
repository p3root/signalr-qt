#include "HubProxy.h"
#include "HubConnection.h"
#include <QtExtJson.h>

HubProxy::HubProxy(HubConnection* connection, QString hubName) : _connection(connection), _hubName(hubName)
{

}

HubProxy::~HubProxy()
{
}

void HubProxy::invoke(QString method, QString param, HubCallback* callback)
{
    QStringList params;
    params.append(param);
    invoke(method, params, callback);

}

void HubProxy::invoke(QString method, QStringList params, HubCallback* callback)
{
    QVariantMap map;
    map.insert("A", params);
    map.insert("I", QString::number(_connection->getCount()));
    map.insert("H", _hubName);
    map.insert("M", method);
    _connection->send(QtExtJson::stringify(QVariant::fromValue(map)), QString::number(_connection->getCount()), callback);
}

void HubProxy::onReceive(QVariant var)
{
    Q_EMIT hubMessageReceived(var);
}

const QString &HubProxy::getName()
{
    return _hubName;
}
