#include "ServerRequest.h"

ServerRequest::ServerRequest(const QHttpRequest &req) : _request(req)
{
    _path = req.path();
    _queryItems = req.url().queryItems();

    //TODO: parse principal
}
