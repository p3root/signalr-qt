#include "HostContext.h"

HostContext::HostContext(const QHttpRequest &req, QHttpResponse &res)
{
    _request = new ServerRequest(req);
    _response = new ServerResponse(res);
}
