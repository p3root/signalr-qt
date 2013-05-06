#ifndef CONNECTIONHELPER_H
#define CONNECTIONHELPER_H

#include <HttpRequest.h>
#include <HttpResponse.h>

class ConnectionHelper
{
public:
    ConnectionHelper();

    static HttpResponse* processRequest(HttpRequest* req);

private:
    static HttpResponse* processNegotiationRequest(HttpRequest* req);
    static HttpResponse* processPingRequest(HttpRequest* req);

    static bool isNegotiationRequest(HttpRequest* req);
    static bool isPingRequest(HttpRequest* req);

    static HttpResponse* errorResponse(int statusCode, QString errorMessage);

};

#endif // CONNECTIONHELPER_H
