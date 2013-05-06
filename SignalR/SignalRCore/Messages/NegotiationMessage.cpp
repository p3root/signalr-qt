#include "NegotiationMessage.h"
#include <QUuid>

NegotiationMessage::NegotiationMessage()
{
}

QString NegotiationMessage::generate(HttpRequest* req)
{
    QVariantMap map;

    QString connectionId = QUuid::createUuid().toString();
    QString connectionToken = connectionId + ":" + ""; //add user context

    map.insert("Url", req->getUrl().toString().replace("/negotiate",""));
    map.insert("ConnectionToken", QByteArray().append(connectionToken).toBase64());
    map.insert("ConnectionId", connectionId);
    map.insert("KeepAliveTimeout", "120");
    map.insert("DisconnectTimeout", "120");
    map.insert("TryWebSockets", "false");
    map.insert("WebSocketsUrl", "");
    map.insert("ProtocolVersion", "1.2");

    return QtExtJson::stringify(QVariant::fromValue(map));

}
