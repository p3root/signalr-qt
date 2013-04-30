#include "TransportHelper.h"
#include <QtGlobal>
#include "Helper.h"


TransportHelper::TransportHelper(void)
{
}


TransportHelper::~TransportHelper(void)
{
}

void TransportHelper::getNegotiationResponse(HttpClient* httpClient, Connection* connnection, ClientTransport::NEGOTIATE_CALLBACK negotiateCallback, void* state)
{
    QString url = connnection->getUrl() + "/negotiate";

    NegotiationRequestInfo* info = new NegotiationRequestInfo();
    info->userState = state;
    info->callback = negotiateCallback;

    httpClient->get(url, &TransportHelper::onNegotiateHttpResponse, info);
}


void TransportHelper::onNegotiateHttpResponse(const QString& httpResponse, SignalException* error, void* state)
{
    Q_UNUSED(error);
    NegotiationRequestInfo* negotiateInfo = (NegotiationRequestInfo*)state;


    if(!error)
    {
        NegotiateResponse response = NegotiateResponse();
        QVariant var = QtExtJson::parse(httpResponse);
        if(var.convert(QVariant::Map))
        {
            QVariantMap map = var.value<QVariantMap>();

            response.connectionId = map.value("ConnectionId").toString();
            response.connectionToken = map.value("ConnectionToken").toString();
            response.protocolVersion = map.value("ProtocolVersion").toString();
        }
        else
        {
            error = new SignalException("Invalid Response type", SignalException::InvalidNegotiationValues);
        }
        negotiateInfo->callback(&response, error, negotiateInfo->userState);
    }
    else
    {
        ((Connection*)negotiateInfo->userState)->changeState(Connection::Connecting, Connection::Disconnected);
        negotiateInfo->callback(0, error, negotiateInfo->userState);
    }

    delete negotiateInfo;
}

QString TransportHelper::getReceiveQueryString(Connection* connection, QString data, QString transport)
{
    QString qs = "?transport=" + transport + "&connectionToken=" + connection->getConnectionToken();//Helper::encode(connection->GetConnectionToken());

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
