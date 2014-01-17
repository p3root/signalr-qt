#ifndef HTTPCONTEXT_H
#define HTTPCONTEXT_H

#include "ServerRequest.h"
#include "ServerResponse.h"

#include <QHttpRequest.h>
#include <QHttpResponse.h>

class HostContext
{
public:
    HostContext(const QHttpRequest &req, QHttpResponse &res);

    ServerRequest &getRequest() { return *_request; }
    ServerResponse &getResponse() { return *_response; }

private:
    ServerRequest *_request;
    ServerResponse *_response;
};

#endif // HTTPCONTEXT_H
