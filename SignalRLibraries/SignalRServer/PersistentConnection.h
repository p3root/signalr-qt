/*
 *  Copyright (c) 2013-2014, p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at)
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

#ifndef PERSISTENTCONNECTION_H
#define PERSISTENTCONNECTION_H

#include <QHttpServerFwd.h>
#include <QHttpResponse.h>

#include "Infrastructure/IProtectedData.h"
#include "Infrastructure/HostContext.h"
#include "Configuration/ConfigurationManager.h"

namespace P3 { namespace SignalR { namespace Server {

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

}}}
#endif // PERSISTENTCONNECTION_H
