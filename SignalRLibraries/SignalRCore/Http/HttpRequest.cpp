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

#include "HttpRequest.h"
#include <QStringList>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
#include <QUrlQuery>
#endif

#define HTTP_HEADER_SEPERATOR "\r\n"
HttpRequest::HttpRequest()
{
}

HttpRequest::~HttpRequest()
{

}

HttpRequest *HttpRequest::parse(QString data)
{
    //TODO: Parse Http Stream
    HttpRequest* req = new HttpRequest();

    QStringList lst = data.split(HTTP_HEADER_SEPERATOR);

    foreach(QString h, lst)
    {
        if(h.isEmpty())
            continue;

        if(h.startsWith("GET"))
        {
            req->setMethod("GET");
            req->setQuery(h.split(' ')[1]);
        }
        else if(h.startsWith("POST"))
        {
            req->setMethod("POST");
            req->setQuery(h.split(' ')[1]);
        }
        else
        {
            req->addHeader(h.split(':')[0], QString(h.split(':')[1]).simplified());
        }
    }

    return req;
}

void HttpRequest::setQuery(QString url)
{


#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
    QUrl urlq(url);
    QUrlQuery qurl(urlq.query());
#else
   QUrl qurl(url);
#endif

    for(int i = 0; i < qurl.queryItems().count(); i++)
    {
        addQuery(qurl.queryItems()[i].first, qurl.queryItems()[i].second);
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
    urlq.setQuery(qurl);
    setUrl(urlq);
#else
   setUrl(qurl);
#endif



}
