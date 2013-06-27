#ifndef HUBCONNECTION_H
#define HUBCONNECTION_H

#include "Connection.h"
#include <QtCore>
#include "HubProxy.h"
#include <QStringList>
#include <QsLog.h>
#include "HubCallback.h"

class HubConnection : public Connection
{
public:
    HubConnection(QString url);
    ~HubConnection();

    void send(QString data, QString id, HubCallback*);
    void stop();

    HubProxy* createHubProxy(QString name);

    QString onSending();
    void onReceived(QVariant data);

    HubProxy* getByName(const QString& name);

private:
    QMap<QString, HubProxy*> _hubs;
    QMap<QString, HubCallback*> _callbacks;

};

#endif // HUBCONNECTION_H
