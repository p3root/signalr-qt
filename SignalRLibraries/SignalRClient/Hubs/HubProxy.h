#ifndef HUBPROXY_H
#define HUBPROXY_H

#include <QObject>
#include <QString>
#include <QVariant>

class HubConnection;

class HubProxy : public QObject
{
    Q_OBJECT
public:
    HubProxy(HubConnection* connection, QString hubName);
    ~HubProxy();

    void invoke(QString method, QString param);
    void invoke(QString method, QStringList params);

    void onReceive(QVariant var);

    const QString& getName();

Q_SIGNALS:
    void hubMessageReceived(QVariant var);

private:
    HubConnection* _connection;
    const QString _hubName;
};

#endif // HUBPROXY_H
