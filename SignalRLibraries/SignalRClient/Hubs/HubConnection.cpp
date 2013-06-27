#include "HubConnection.h"
#include <QtExtJson.h>

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
        lst.append(map);
    }
    

    QString json = QtExtJson::stringify(QVariant::fromValue(lst));
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
    return _hubs[name];
}
