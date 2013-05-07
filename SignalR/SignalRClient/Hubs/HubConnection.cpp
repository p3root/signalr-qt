#include "HubConnection.h"
#include <QtExtJson.h>

HubConnection::HubConnection(QString url, ConnectionHandler *handler) : Connection(url, handler)
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

void HubConnection::stop()
{
    Connection::stop();
}

HubProxy *HubConnection::createHubProxy(QString name)
{
    if(_state != Disconnected)
    {
        QLOG_ERROR() << "A HubProxy cannot be added after the connection has been started.";
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
    }
    lst.append(map);

    QString json = QtExtJson::stringify(QVariant::fromValue(lst));
    return json;
}

const HubProxy &HubConnection::getByName(const QString &name)
{
    return *_hubs[name];
}
