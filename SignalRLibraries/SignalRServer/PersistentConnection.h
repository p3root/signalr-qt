#ifndef PERSISTENTCONNECTION_H
#define PERSISTENTCONNECTION_H

#include <QHttpServerFwd.h>
#include <QHttpResponse.h>

#include "Infrastructure/IProtectedData.h"
#include "Infrastructure/HostContext.h"
#include "Configuration/ConfigurationManager.h"


class PersistentConnection
{
public:
    PersistentConnection();

    void processRequest(const QHttpRequest &req, QHttpResponse &res);
    void processRequest(HostContext *context);

    ConfigurationManager &getConfigurationManager();
    void setConfigurationManager(ConfigurationManager *manager);

    virtual bool authorizeRequest(const HostContext &context);

protected:
    void processNegotiationRequest(HostContext &context);
    void processPingRequest(HostContext &context);
    QList<QString> verifyGroups(HostContext &context, const QString &connectionId);

    QList<QString> getSignals(const QString &userId, const QString &connectionId);

    QList<QString> onRejoiningGroups(const ServerRequest &request, const QList<QString> &groups, const QString &connectionId);

    static bool isNegotiationReqeust(HostContext &context);
    static bool isPingRequest(HostContext &context);

private:
    QString getConnectionId(HostContext &context, const QString &connectionToken, bool &ok);
    QString getUserIdentity(HostContext &context);

    QList<QString> getDefaultSignals(const QString &userId, const QString &connectionId);
    QList<QString> appendGroupPrefixes(HostContext &context, const QString &connectionId);

private:
    IProtectedData *_protectedData;
    ConfigurationManager *_configurationManager;

};

#endif // PERSISTENTCONNECTION_H
