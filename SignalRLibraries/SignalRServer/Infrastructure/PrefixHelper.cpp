#include "PrefixHelper.h"

const QString PrefixHelper::HubPrefix = "h-";
const QString PrefixHelper::HubGroupPrefix = "hg-";
const QString PrefixHelper::HubConnectionIdPrefix = "hc-";
const QString PrefixHelper::HubUserPrefix = "hu-";

const QString PrefixHelper::PeristentConnectionPrefix = "pc-";
const QString PrefixHelper::PeristentConnectionGroupPrefix = "pcg-";

const QString PrefixHelper::ConnectionIdPrefix = "c-";
const QString PrefixHelper::AckPrefix = "ack-";

QString PrefixHelper::getPersistentConnectionName(const QString &connectionName)
{
    return PeristentConnectionPrefix + QString(connectionName);
}

QString PrefixHelper::getConnectionId(const QString &connectionId)
{
    return ConnectionIdPrefix + QString(connectionId);
}

QString PrefixHelper::getAck(const QString &connectionId)
{
    return AckPrefix + QString(connectionId);
}
