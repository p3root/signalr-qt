#ifndef HUBPROXY_H
#define HUBPROXY_H

#include <QString>

class HubConnection;

class HubProxy
{
public:
    HubProxy(HubConnection* connection, QString hubName);
    ~HubProxy();

    void invoke(QString method, QString param);
    void invoke(QString method, QStringList params);

    const QString& getName();

private:
    HubConnection* _connection;
    const QString _hubName;
};

#endif // HUBPROXY_H
