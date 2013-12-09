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
#include <Connection.h>

HttpClient::HttpClient(Connection *con) : _isAborting(false), _man(0)
{
    _connection = con;
    _man = new QNetworkAccessManager();
    _man->setProxy(_connection->getProxySettings());
    _getMutex = new QMutex(QMutex::Recursive);
    _postMutex = new QMutex(QMutex::Recursive);

    connect(_man, SIGNAL(finished(QNetworkReply*)), SLOT(requestFinished(QNetworkReply*)));
    connect(_man, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(onIgnoreSSLErros(QNetworkReply*,QList<QSslError>)));
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

    QUrl decodedUrl(url);
    QString encodedUrl =decodedUrl.scheme() +"://"+ decodedUrl.host() + ":" + QString::number(decodedUrl.port()) + decodedUrl.path() +"?"+ Helper::getEncodedQueryString(decodedUrl, _connection);
    QUrl reqUrl = QUrl();

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
    reqUrl.setUrl(QByteArray().append(encodedUrl));
#else
    reqUrl.setEncodedUrl(QByteArray().append(encodedUrl));
#endif

    QNetworkRequest req = QNetworkRequest(reqUrl);
    QCryptographicHash::hash("data", QCryptographicHash::Sha1);

    req.setRawHeader("User-Agent", "SignalR-Qt.Client");

    for(int i = 0; i < _connection->getAdditionalHttpHeaders().size(); i++)
    {
        QString first = QString(_connection->getAdditionalHttpHeaders().at(i).first);
        QString second = QString(_connection->getAdditionalHttpHeaders().at(i).second);
        req.setRawHeader(first.toAscii(), second.toAscii());
    }


     _connection->emitLogMessage("starting get request", Connection::Debug);
    QNetworkReply *getReply = _man->get(req);

    _currentConnections.append(getReply);
}

void HttpClient::post(QString url, QMap<QString, QString> arguments)
{
    QMutexLocker l(_postMutex);
    Q_UNUSED(l);

    QString queryString;
    QMap<QString, QString>::iterator it = arguments.begin();
    while(it != arguments.end())
    {
        queryString.append(QString("%1=%2&").arg(it.key(), Helper::encode(it.value())));
        ++it;
    }
    queryString.remove(queryString.length()-1, 1);

    QUrl decodedUrl(url);
    QString encodedUrl =decodedUrl.scheme() +"://"+ decodedUrl.host() + ":" + QString::number(decodedUrl.port()) + decodedUrl.path() +"?"+ Helper::getEncodedQueryString(decodedUrl, _connection);
    QUrl reqUrl = QUrl();

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
    reqUrl.setUrl(QByteArray().append(encodedUrl));
#else
    reqUrl.setEncodedUrl(QByteArray().append(encodedUrl));
#endif

    QNetworkRequest req = QNetworkRequest(reqUrl);
    req.setRawHeader("User-Agent", "SignalR-Qt.Client");
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    for(int i = 0; i < _connection->getAdditionalHttpHeaders().size(); i++)
    {
        QString first = QString(_connection->getAdditionalHttpHeaders().at(i).first);
        QString second = QString(_connection->getAdditionalHttpHeaders().at(i).second);
        req.setRawHeader(first.toAscii(), second.toAscii());
    }

    _connection->emitLogMessage("starting post request", Connection::Debug);

    QNetworkReply *postReply = _man->post(req, QByteArray().append(queryString));

    _currentConnections.append(postReply);
}

void HttpClient::abort()
{
    _isAborting = true;

    foreach(QNetworkReply *reply, _currentConnections)
    {
        reply->abort();
    }
}

void HttpClient::requestFinished(QNetworkReply *reply)
{
    QNetworkReply::NetworkError error = reply->error();
    QNetworkAccessManager::Operation operation = reply->operation();

    if(error == QNetworkReply::OperationCanceledError)
    {
        _currentConnections.removeOne(reply);
        reply->deleteLater();
        return;
    }

    if(operation == QNetworkAccessManager::GetOperation)
    {
        _connection->emitLogMessage("get request finished", Connection::Debug);

        if(error == QNetworkReply::NoError)
            getRequestFinished(reply);
        else
            replyError(error, reply);
    }
    else if(operation == QNetworkAccessManager::PostOperation)
    {
        _connection->emitLogMessage("post request finished", Connection::Debug);

        if(error == QNetworkReply::NoError)
            postRequestFinished(reply);
        else
            replyError(error, reply);
    }
}

void HttpClient::getRequestFinished(QNetworkReply *reply)
{
    if(!reply)
        return;

    QString data = QString(reply->readAll());

    if(reply->error() == QNetworkReply::NoError)
    {
        Q_EMIT getRequestCompleted(data, 0);

        _currentConnections.removeOne(reply);
        reply->deleteLater();
    }
}

void HttpClient::replyError(QNetworkReply::NetworkError err, QNetworkReply *reply)
{
    if(!reply)
        return;

    int error = (int)err;
    QString errorString = reply->errorString();

    _connection->emitLogMessage("request error " + errorString, Connection::Error);

    if(error != QNetworkReply::NoError)
    {
        SignalException* ex = 0;

        switch(error)
        {
            case 1:
                ex = new SignalException(errorString, SignalException::ConnectionRefusedError);
                break;
            case 2:
                ex = new SignalException(errorString, SignalException::RemoteHostClosedConnection);
                break;
            case 3:
                ex = new SignalException(errorString, SignalException::HostNotFoundError);
                break;
            case 4:
                ex = new SignalException(errorString, SignalException::SocketOperationTimedOut);
                break;
            case 5:
                return;
            case 99:
                ex = new SignalException(errorString, SignalException::UnkownNetworkError);
                break;
            case 204:
                ex = new SignalException(errorString, SignalException::ServerRequiresAuthorization);
                break;
            default:
                ex = new SignalException(errorString, SignalException::UnkownError);
                break;
        }

        if(!_isAborting)
        {
            Q_EMIT getRequestCompleted("", ex);
            _currentConnections.removeOne(reply);
            reply->deleteLater();
        }
    }
}

void HttpClient::postRequestFinished(QNetworkReply *reply)
{
    if(!reply)
        return;

    QString data = QString(reply->readAll());

    if( reply->error() == QNetworkReply::NoError)
    {
        Q_EMIT postRequestCompleted(data, 0);
        _currentConnections.removeOne(reply);
        reply->deleteLater();
    }
}

void HttpClient::onIgnoreSSLErros(QNetworkReply *reply, QList<QSslError> error)
{
    Q_UNUSED(error);
    reply->ignoreSslErrors(error);
}
