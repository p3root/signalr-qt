/*
 *  Copyright (c) 2013, p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at)
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *
 *    Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright notice, this
 *    list of conditions and the following disclaimer in the documentation and/or
 *    other materials provided with the distribution.
 *
 *    Neither the name of the {organization} nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
