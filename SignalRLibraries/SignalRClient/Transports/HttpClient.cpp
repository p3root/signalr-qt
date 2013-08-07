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

#include "HttpClient.h"

#if defined(Q_OS_QNX)
	#include <QtNetwork/qnetworkrequest.h>
	#include <QtNetwork/qnetworkreply.h>
#else
	#include <QNetworkRequest>
	#include <QNetworkReply>
#endif

#include <QEventLoop>
#include "Helper/Helper.h"
#include <unistd.h>

HttpClient::HttpClient() : _isAborting(false), _getReply(0), _postReply(0), _man(0)
{
    _man = new QNetworkAccessManager();
    _getMutex = new QMutex(QMutex::Recursive);
    _postMutex = new QMutex(QMutex::Recursive);
}

HttpClient::~HttpClient()
{
    delete _man;
    delete _getMutex;
    delete _postMutex;
}

void HttpClient::get(QString url)
{
    QMutexLocker l(_getMutex);
    Q_UNUSED(l);
    if(_getReply)
    {
        _getReply->deleteLater();
        _getReply = 0;
    }

    QUrl decodedUrl(url);
    QString encodedUrl =decodedUrl.scheme() +"://"+ decodedUrl.host() + ":" + QString::number(decodedUrl.port()) + decodedUrl.path() +"?"+ Helper::getEncodedQueryString(decodedUrl);
    QUrl reqUrl = QUrl();

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
    reqUrl.setUrl(QByteArray().append(encodedUrl));
#else
    reqUrl.setEncodedUrl(QByteArray().append(encodedUrl));
#endif

    QNetworkRequest req = QNetworkRequest(reqUrl);
    req.setRawHeader("User-Agent", "SignalR-Qt.Client");
    _getReply = _man->get(req);

    connect(_getReply, SIGNAL(finished()), this, SLOT(getRequestFinished()), Qt::AutoConnection);
    connect(_getReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(getError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
}

void HttpClient::post(QString url, QMap<QString, QString> arguments)
{
    QMutexLocker l(_postMutex);
    Q_UNUSED(l);
    if(_postReply)
    {
        _postReply->deleteLater();
        _postReply = 0;
    }

    QString queryString;
    QMap<QString, QString>::iterator it = arguments.begin();
    while(it != arguments.end())
    {
        queryString.append(QString("%1=%2&").arg(it.key(), Helper::encode(it.value())));
        ++it;
    }
    queryString.remove(queryString.length()-1, 1);

    QUrl decodedUrl(url);
    QString encodedUrl =decodedUrl.scheme() +"://"+ decodedUrl.host() + ":" + QString::number(decodedUrl.port()) + decodedUrl.path() +"?"+ Helper::getEncodedQueryString(decodedUrl);
    QUrl reqUrl = QUrl();

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
    reqUrl.setUrl(QByteArray().append(encodedUrl));
#else
    reqUrl.setEncodedUrl(QByteArray().append(encodedUrl));
#endif

    QNetworkRequest req = QNetworkRequest(reqUrl);
    req.setRawHeader("User-Agent", "SignalR-Qt.Client");
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    _postReply = _man->post(req, QByteArray().append(queryString));

    connect(_postReply, SIGNAL(finished()), this, SLOT(postRequestFinished()), Qt::AutoConnection);
    connect(_postReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(postError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
}

void HttpClient::abort()
{
    _isAborting = true;

    if(_getReply)
        _getReply->abort();
    if(_postReply)
        _postReply->abort();
}


void HttpClient::getRequestFinished()
{
    QString data = QString(_getReply->readAll());

    if( _getReply->error() == QNetworkReply::NoError)
    {
        Q_EMIT getRequestCompleted(data, 0);
    }
}

void HttpClient::getError(QNetworkReply::NetworkError)
{
    int error = _getReply->error();
    QString errorString = _getReply->errorString();

    if(error != QNetworkReply::NoError)
    {
        SignalException* ex = 0;
        switch(error)
        {
        case 1:
            ex = new SignalException(errorString, SignalException::ConnectionRefusedError);
            break;
        default:
            ex = new SignalException(errorString, SignalException::UnkownError);
            break;
        }

        if(!_isAborting)
            Q_EMIT getRequestCompleted("", ex);
    }
}

void HttpClient::postRequestFinished()
{
    QString data = QString(_postReply->readAll());

    if( _postReply->error() == QNetworkReply::NoError)
    {
        Q_EMIT postRequestCompleted(data, 0);
    }
}

void HttpClient::postError(QNetworkReply::NetworkError)
{
    int error = _postReply->error();
    QString errorString = _postReply->errorString();

    if(error != QNetworkReply::NoError)
    {
        SignalException* ex = 0;
        switch(error)
        {
        case 1:
            ex = new SignalException(errorString, SignalException::ConnectionRefusedError);
            break;
        default:
            ex = new SignalException(errorString, SignalException::UnkownError);
            break;
        }

        if(!_isAborting)
            Q_EMIT postRequestCompleted("", ex);
    }
}
