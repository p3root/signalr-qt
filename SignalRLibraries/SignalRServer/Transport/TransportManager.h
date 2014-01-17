#ifndef TRANSPORTMANAGER_H
#define TRANSPORTMANAGER_H

#include <QMap>
#include <QString>
#include "TransportBase.h"
#include "../Infrastructure/HostContext.h"

class TransportManager
{
public:
    TransportManager();


    static bool transportExists(const QString &transportName);
    static TransportBase *createTransport(const QString &transportName, HostContext &context);


};

#endif // TRANSPORTMANAGER_H
