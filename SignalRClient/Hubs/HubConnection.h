#ifndef HUBCONNECTION_H
#define HUBCONNECTION_H

#include "Connection.h"
#include <QtCore>
#include "HubProxy.h"
#include <QStringList>
#include <QsLog.h>

class HubConnection : public Connection
{
public:
    HubConnection(QString url, ConnectionHandler* handler);
    ~HubConnection();

    void stop();

    HubProxy* createHubProxy(QString name);

    QString onSending();

private:
    QMap<QString, HubProxy*> _hubs;

};

#endif // HUBCONNECTION_H
