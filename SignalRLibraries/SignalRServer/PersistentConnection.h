#ifndef PERSISTENTCONNECTION_H
#define PERSISTENTCONNECTION_H

#include <QHttpServerFwd.h>
#include <Infrastructure/IProtectedData.h>
#include "Configuration/ConfigurationManager.h"

class PersistentConnection
{
public:
    PersistentConnection();

    void processRequest(const QHttpRequest &req, QHttpResponse &res);

    ConfigurationManager &getConfigurationManager();
    void setConfigurationManager(ConfigurationManager *manager);

protected:
    void processNegotiationRequest(const QHttpRequest &req, QHttpResponse &res);
    void processPingRequest(const QHttpRequest &req, QHttpResponse &res);

    static bool isNegotiationReqeust(const QHttpRequest &req);
    static bool isPingRequest(const QHttpRequest &req);

private:
    IProtectedData *_protectedData;
    ConfigurationManager *_configurationManager;

};

#endif // PERSISTENTCONNECTION_H
