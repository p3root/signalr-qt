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
    Q_UNUSED(connection);
    Q_UNUSED(raw);
    Q_UNUSED(timedOut);
    Q_UNUSED(disconnected);


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

        if(*disconnected)
            return;

        //its a hub message, dispatch it to the correct hub
        if(map.contains("H"))
        {

        }

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
    }
    else
    {
        return 0;
    }
    return response;
}
