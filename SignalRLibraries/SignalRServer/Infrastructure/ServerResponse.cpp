#include "ServerResponse.h"

ServerResponse::ServerResponse(QHttpResponse &res) : _response(res)
{
}

void ServerResponse::write(const QString &data)
{
    write(QByteArray(data.toAscii().constData(), data.size()));
}

void ServerResponse::write(const QByteArray &data)
{
    _data.append(data);
}

void ServerResponse::setContentType(QString contentType)
{
    _contentType = contentType;
    _response.setHeader("Content-Type", _contentType);
}

void ServerResponse::closeResponse(int status)
{
    _response.writeHead(status);
    _response.end(_data);
}
