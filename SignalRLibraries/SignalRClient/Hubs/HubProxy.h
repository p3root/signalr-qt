#ifndef HUBPROXY_H
#define HUBPROXY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include "HubCallback.h"

class HubConnection;

class HubProxy : public QObject
{
    Q_OBJECT
public:
    HubProxy(HubConnection* connection, QString hubName);
    ~HubProxy();

    void invoke(QString method, QString param, HubCallback*);
    void invoke(QString method, QStringList params,HubCallback*);

    void onReceive(QVariant var);

    const QString& getName();

Q_SIGNALS:
    void hubMessageReceived(QVariant var);

private:
    HubConnection* _connection;
    const QString _hubName;
};

#endif // HUBPROXY_H
