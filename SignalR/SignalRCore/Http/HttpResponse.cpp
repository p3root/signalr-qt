#include "HttpResponse.h"

HttpResponse::HttpResponse(void) : QObject()
{
}

HttpResponse::~HttpResponse()
{
}

const QByteArray HttpResponse::getHttpResponse()
{

    QByteArray ret;

    ret += "HTTP/1.1 " + QString::number(_statusCode) + getStatusString() +" \r\n";

    for(int i = 0; i < _headers.count(); i++)
    {
        ret  += _headers.keys()[i] + ": " + _headers.values()[i] + "\r\n";
    }

    ret  += "\r\n";
    ret  += _content;

    return ret;
}

QString HttpResponse::getStatusString()
{
    switch(_statusCode)
    {
    case 200:
        return "OK";
    case 500:
        return "Internal Server Error";

    default:
        return "";
    }
}
