#include "ConnectionHelper.h"
#include <QUrl>
#include "Messages/NegotiationMessage.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
#include <QUrlQuery>
#endif

ConnectionHelper::ConnectionHelper()
{
}

HttpResponse *ConnectionHelper::processRequest(HttpRequest *req)
{
    if(!req)
        return 0;

    if(isNegotiationRequest(req))
    {
        return processNegotiationRequest(req);
    }
    else if(isPingRequest(req))
    {
        return processPingRequest(req);
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
    QString connectionToken = QUrlQuery(req->getUrl().query()).queryItemValue("connectionToken");
#else
    QString connectionToken = req->getUrl().queryItemValue("connectionToken");
#endif

    if(connectionToken.isEmpty())
        return errorResponse(500, "No connection token found in query string");


    return errorResponse(500, "Unkown error occured");
}

bool ConnectionHelper::isNegotiationRequest(HttpRequest *req)
{
    return req->getUrl().path().endsWith("/negotiate");
}

bool ConnectionHelper::isPingRequest(HttpRequest *req)
{
    return req->getUrl().path().endsWith("/ping");
}

HttpResponse *ConnectionHelper::errorResponse(int statusCode, QString errorMessage)
{
    HttpResponse* res = new HttpResponse();

    res->addHeader("Server", "SignalR.P3 v0.1");
    res->addHeader("Content-Length:", QString::number(errorMessage.length()));
    res->addHeader("Content-Type:", "text/plain");
    res->setStatusCode(statusCode);
    res->setContent(errorMessage);

    return res;
}

HttpResponse* ConnectionHelper::processNegotiationRequest(HttpRequest *req)
{
    HttpResponse* res = new HttpResponse();
    QString json = NegotiationMessage::generate(req);

    res->addHeader("Server", "SignalR.P3 v0.1");
    res->addHeader("Content-Length:", QString::number(json.length()));
    res->addHeader("Content-Type:", "application/json");
    res->setStatusCode(200);
    res->setContent(json);

    return res;
}

HttpResponse *ConnectionHelper::processPingRequest(HttpRequest *)
{
    HttpResponse* res = new HttpResponse();
    QString json = "{\"Response\": \"pong\"}";

    res->addHeader("Server", "SignalR.P3 v0.1");
    res->addHeader("Content-Length:", QString::number(json.length()));
    res->addHeader("Content-Type:", "application/json");
    res->setStatusCode(200);
    res->setContent(json);

    return res;
}
