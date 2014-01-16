#include "PersistentConnection.h"
#include <QHttpRequest.h>
#include <QHttpResponse.h>
#include <QMap>
#include <QUuid>
#include <QextJson.h>

#include "Infrastructure/EmptyProtectionData.h"
#include "Infrastructure/Purpose.h"

#define NEGOTIATE_PATH "/negotiate"
#define PING_PATH "/ping"

PersistentConnection::PersistentConnection() : _protectedData(0), _configurationManager(0)
{
    _protectedData = new EmptyProtectionData();
    _configurationManager = new ConfigurationManager();
}

void PersistentConnection::processRequest(const QHttpRequest &req, QHttpResponse &res)
{
    if(isNegotiationReqeust(req))
    {
        processNegotiationRequest(req, res);
        return;
    }
    else if(isPingRequest(req))
    {
        processPingRequest(req, res);
        return;
    }
}

ConfigurationManager &PersistentConnection::getConfigurationManager()
{
    return *_configurationManager;
}

void PersistentConnection::setConfigurationManager(ConfigurationManager *manager)
{
    if(_configurationManager)
        delete _configurationManager;

    _configurationManager = manager;
}

void PersistentConnection::processNegotiationRequest(const QHttpRequest &req, QHttpResponse &res)
{
    Q_UNUSED(req);
    Q_UNUSED(res);

    QVariantMap payload;
    QString connectionId = QUuid::createUuid().toString().replace("{", "").replace("}", "");
    QString connectionToken = _protectedData->protect(connectionId, Purpose::ConnectionToken);

    QString path = QString(req.path());
    payload.insert("Url", path.replace(NEGOTIATE_PATH, ""));
    payload.insert("ConnectionToken", connectionToken);
    payload.insert("ConnectionId", connectionId);
    payload.insert("KeepAliveTimeout", _configurationManager->getKeepAliveTimeout());
    payload.insert("DisconnectTimeout", _configurationManager->getDisconnectTimeout());
    payload.insert("TryWebSockets", true);
    payload.insert("ProtocolVersion", "1.3");
    payload.insert("TransportConnectTimeout", _configurationManager->getTransportConnectTimeout());


    QString json = QextJson::stringify(QVariant::fromValue(payload));

    res.writeHead(QHttpResponse::STATUS_OK);
    res.write(json);
    res.end();
}

void PersistentConnection::processPingRequest(const QHttpRequest &req, QHttpResponse &res)
{
    Q_UNUSED(req);
    res.writeHead(QHttpResponse::STATUS_OK);
    res.write(QByteArray("pong"));
    res.end();
}

bool PersistentConnection::isNegotiationReqeust(const QHttpRequest &req)
{
    return req.path().endsWith(NEGOTIATE_PATH);
}

bool PersistentConnection::isPingRequest(const QHttpRequest &req)
{
    return req.path().endsWith(PING_PATH);
}
