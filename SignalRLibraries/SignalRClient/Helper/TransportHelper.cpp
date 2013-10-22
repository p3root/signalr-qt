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
 *    Neither the name of the {organization} nor the names of its
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

#include "TransportHelper.h"
#include <QtGlobal>
#include "Helper.h"


TransportHelper::TransportHelper(void)
{
}


TransportHelper::~TransportHelper(void)
{
}

QString TransportHelper::getReceiveQueryString(Connection* connection, QString data, QString transport)
{
    QString qs = "?transport=" + transport + "&connectionToken=" + connection->getConnectionToken();

    QString messageId = connection->getMessageId();
    QString groupsToken = connection->getGroupsToken();
    
    if(!messageId.isEmpty())
    {
        qs += "&messageId=" + messageId;
    }

    if(!groupsToken.isEmpty())
    {
        qs += "&groupsToken=" + groupsToken;
    }

    if(!data.isEmpty())
    {
        qs += "&connectionData=" + data;
    }

    return qs;
}

void TransportHelper::processMessages(Connection* connection, QString raw, bool* timedOut, bool* disconnected)
{
    connection->updateLastKeepAlive();

    QVariant var = QtExtJson::parse(raw);
    if(var.convert(QVariant::Map))
    {
        QVariantMap map = var.value<QVariantMap>();

        if(map.contains("T"))
        {
            if(map["T"].value<int>() == 1)
            {
                *timedOut = true;
            }
        }

        if(map.contains("D"))
        {
            if(map["D"].value<int>() == 1)
            {
                *disconnected = true;
            }
        }

        if(map.contains("I"))
        {
            connection->onReceived(var);
        }

        if(map.contains("G"))
        {
            connection->setGroupsToken(map["G"].toString());
        }

        if(map.contains("C"))
        {
            connection->setMessageId(map["C"].toString());
        }

        if(*disconnected)
            return;

        if(map.contains("M"))
        {
            if(map["M"].convert(QVariant::List))
            {
                QVariantList lst = map["M"].value<QVariantList>();

                foreach(QVariant cur, lst)
                {
                    connection->onReceived(cur);
                }
            }
        }
    }
}

const NegotiateResponse* TransportHelper::parseNegotiateHttpResponse(const QString &httpResponse)
{
    NegotiateResponse* response = new NegotiateResponse();
    QVariant var = QtExtJson::parse(httpResponse);
    if(var.convert(QVariant::Map))
    {
        QVariantMap map = var.value<QVariantMap>();

        response->connectionId = map.value("ConnectionId").toString();
        response->connectionToken = map.value("ConnectionToken").toString();
        response->protocolVersion = map.value("ProtocolVersion").toString();

        if(map.contains("KeepAliveTimeout"))
            response->keepAliveTimeout = map.value("KeepAliveTimeout").toDouble();
        else
            response->keepAliveTimeout = -1;

        if(map.contains("DisconnectTimeout"))
            response->disconnectTimeout = map.value("DisconnectTimeout").toDouble();
        else
            response->disconnectTimeout = -1;
    }
    else
    {
        return 0;
    }
    return response;
}
