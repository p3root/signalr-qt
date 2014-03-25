/*
 *  Copyright (c) 2013, p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at)
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *
 *    Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright notice, this
 *    list of conditions and the following disclaimer in the documentation and/or
 *    other materials provided with the distribution.
 *
 *    Neither the name of the p3root - Patrik Pfaffenbauer nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "PersistentConnection.h"
#include <QHttpRequest.h>
#include <QHttpResponse.h>
#include <QMap>
#include <QUuid>
#include <QextJson.h>
#include <QStringList>

#include "Infrastructure/EmptyProtectionData.h"
#include "Infrastructure/Purpose.h"
#include "Infrastructure/PrefixHelper.h"

#include "Transport/TransportManager.h"
#include "Transport/TransportBase.h"

#define NEGOTIATE_PATH "/negotiate"
#define PING_PATH "/ping"

namespace P3 { namespace SignalR { namespace Server {

PersistentConnection::PersistentConnection() : _protectedData(0), _configurationManager(0)
{
    _protectedData = new EmptyProtectionData();
    _configurationManager = new ConfigurationManager();
}

void PersistentConnection::processRequest(const QHttpRequest &req, QHttpResponse &res)
{
    HostContext *context = new HostContext(req, res);

    // Add the nosniff header for all responses to prevent IE from trying to sniff mime type from contents
    res.setHeader("X-Content-Type-Options", "nosniff");

    if(authorizeRequest(*context))
    {
        processRequest(context);
        return;
    }

    if(context->getRequest().getUser().getIsAuthenticated())
    {
        // If the user is authenticated and authorize failed then 403
        context->getResponse().closeResponse(403);
    }
    else
    {
        // If we failed to authorize the request then return a 401
        context->getResponse().closeResponse(401);
    }
}

void PersistentConnection::processRequest(HostContext *context)
{
    if(isNegotiationReqeust(*context))
    {
        processNegotiationRequest(*context);
        return;
    }
    else if(isPingRequest(*context))
    {
        processPingRequest(*context);
        return;
    }
    QString value = context->getRequest().getUrl().queryItemValue("transport");
    if(value.isEmpty())
    {
        context->getResponse().closeResponse(500);
        return;
    }
    if(TransportManager::transportExists(value))
    {
        TransportBase *t = TransportManager::createTransport(value, *context);

        QString connectionToken = context->getRequest().getUrl().queryItemValue("connectionToken");

        if(connectionToken.isEmpty())
        {
            context->getResponse().closeResponse(500);
            return;
        }

        bool ok;
        QString connectionId = getConnectionId(*context, connectionToken, ok);

        if(!ok)
        {
            context->getResponse().closeResponse(401);
            return;
        }

        t->setConnectionId(connectionId);

        QList<QString> signal = getSignals(context->getRequest().getUser().getName(), connectionId);
        QList<QString> groups = appendGroupPrefixes(*context, connectionId);


    }
    else
    {
        context->getResponse().closeResponse(500);
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

bool PersistentConnection::authorizeRequest(const HostContext &context)
{
    Q_UNUSED(context);
    return true;
}

void PersistentConnection::processNegotiationRequest(HostContext &context)
{
    Q_UNUSED(context);

    QVariantMap payload;
    QString connectionId = QUuid::createUuid().toString().replace("{", "").replace("}", "");
    QString connectionToken = _protectedData->protect(connectionId, Purpose::ConnectionToken);

    QString path = QString(context.getRequest().getLocalPath());
    payload.insert("Url", path.replace(NEGOTIATE_PATH, ""));
    payload.insert("ConnectionToken", connectionToken);
    payload.insert("ConnectionId", connectionId);
    payload.insert("KeepAliveTimeout", _configurationManager->getKeepAliveTimeout());
    payload.insert("DisconnectTimeout", _configurationManager->getDisconnectTimeout());
    payload.insert("TryWebSockets", true);
    payload.insert("ProtocolVersion", "1.3");
    payload.insert("TransportConnectTimeout", _configurationManager->getTransportConnectTimeout());


    QString json = QextJson::stringify(QVariant::fromValue(payload));

    context.getResponse().write(json);
    context.getResponse().closeResponse(200);
}

void PersistentConnection::processPingRequest(HostContext &context)
{
    Q_UNUSED(context);
    context.getResponse().getResponse().write(QByteArray("pong"));
    context.getResponse().closeResponse(200);
}

QList<QString> PersistentConnection::verifyGroups(HostContext &context, const QString &connectionId)
{
    QString groupsToken = context.getRequest().getUrl().queryItemValue("groupsToken");

    if(groupsToken.isEmpty())
    {
        return QList<QString>();
    }

    QString unprotectedGroupsToken = _protectedData->unprotect(groupsToken, Purpose::Groups);

    if(unprotectedGroupsToken.isEmpty())
    {
        return QList<QString>();
    }

    QStringList tokens = unprotectedGroupsToken.split(":");

    QString groupsConnectionId = tokens[0];
    QString groupsValue = tokens.count() > 1 ? tokens[1] : "";

    if(groupsConnectionId != connectionId)
    {
        return QList<QString>();
    }

    return QextJson::parse(groupsValue).value<QStringList>();
}

QList<QString> PersistentConnection::getSignals(const QString &userId, const QString &connectionId)
{
    return getDefaultSignals(userId, connectionId);
}

QList<QString> PersistentConnection::onRejoiningGroups(const ServerRequest &request, const QList<QString> &groups, const QString &connectionId)
{
    Q_UNUSED(request);
    Q_UNUSED(connectionId);

    return groups;
}

bool PersistentConnection::isNegotiationReqeust(HostContext &context)
{
    return context.getRequest().getLocalPath().endsWith(NEGOTIATE_PATH);
}

bool PersistentConnection::isPingRequest(HostContext &context)
{
    return context.getRequest().getLocalPath().endsWith(PING_PATH);
}

QString PersistentConnection::getConnectionId(HostContext &context, const QString &connectionToken, bool &ok)
{
    QString unprotectedConnectionToken = "";
    ok = true;

    unprotectedConnectionToken = _protectedData->unprotect(connectionToken, Purpose::ConnectionToken);

    QStringList tokens = unprotectedConnectionToken.split(":");

    QString connectionId = tokens[0];
    QString tokenUserName = tokens.count() > 1 ? tokens[0] : "";
    QString userName = getUserIdentity(context);

    if(tokenUserName != userName)
    {
        ok = false;
        return "";
    }

    return connectionId;
}

QString PersistentConnection::getUserIdentity(HostContext &context)
{
    if(context.getRequest().getUser().getIsAuthenticated())
        return context.getRequest().getUser().getName();
    return "";
}

QList<QString> PersistentConnection::getDefaultSignals(const QString &userId, const QString &connectionId)
{
    QList<QString> ret;

    Q_UNUSED(userId);
    QString defaultSignal = "PERSISTENT_CONNECTION";
    ret.append(PrefixHelper::getPersistentConnectionName(defaultSignal));
    ret.append(PrefixHelper::getConnectionId(connectionId));
    ret.append(PrefixHelper::getAck(connectionId));

    return ret;
}

QList<QString> PersistentConnection::appendGroupPrefixes(HostContext &context, const QString &connectionId)
{
    QList<QString> lst = onRejoiningGroups(context.getRequest(), verifyGroups(context, connectionId), connectionId);
    QList<QString> withPrefix;

    foreach(QString str, lst)
    {
        withPrefix.append(PrefixHelper::PeristentConnectionGroupPrefix + str);
    }
    return withPrefix;

}

}}}
