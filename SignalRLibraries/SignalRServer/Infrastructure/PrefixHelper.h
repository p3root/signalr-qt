#ifndef PREFIXHELPER_H
#define PREFIXHELPER_H

#include <QString>

class PrefixHelper
{
public:
    static QString getPersistentConnectionName(const QString &connectionName);
    static QString getConnectionId(const QString &connectionId);
    static QString getAck(const QString &connectionId);

    static const QString HubPrefix;
    static const QString HubGroupPrefix;
    static const QString HubConnectionIdPrefix;
    static const QString HubUserPrefix;

    static const QString PeristentConnectionPrefix;
    static const QString PeristentConnectionGroupPrefix;

    static const QString ConnectionIdPrefix;
    static const QString AckPrefix;
};

#endif // PREFIXHELPER_H
