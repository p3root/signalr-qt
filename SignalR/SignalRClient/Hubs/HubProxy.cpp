#include "HubProxy.h"
#include "HubConnection.h"
#include <QtExtJson.h>

HubProxy::HubProxy(HubConnection* connection, QString hubName) : _connection(connection), _hubName(hubName)
{

}

HubProxy::~HubProxy()
{
}

void HubProxy::invoke(QString method, QString param)
{
    QStringList params;
    params.append(param);
    invoke(method, params);

}

void HubProxy::invoke(QString method, QStringList params)
{
    QVariantMap map;
    map.insert("A", params);
    map.insert("I", QString::number(_connection->getCount()));
    map.insert("H", _hubName);
    map.insert("M", method);
    _connection->send(QtExtJson::stringify(QVariant::fromValue(map)));
}

const QString &HubProxy::getName()
{
    return _hubName;
}
