/*
 *  Copyright (c) 2013-2014, p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at)
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
#include <Connection_p.h>

namespace P3 { namespace SignalR { namespace Client {

HttpClient::HttpClient(ConnectionPrivate *con) : _isAborting(false), _man(0)
{
    _connection = con;
    _man = new QNetworkAccessManager();

#ifndef QT_NO_NETWORKPROXY
    _man->setProxy(_connection->getProxySettings());
#endif

    qRegisterMetaType<QMap<QString,QString> >("QMap<QString,QString>");

    _getMutex = new QMutex(QMutex::Recursive);
    _postMutex = new QMutex(QMutex::Recursive);
    _connectionLock = new QMutex(QMutex::Recursive);

    connect(_man, SIGNAL(finished(QNetworkReply*)), SLOT(requestFinished(QNetworkReply*)));
#ifndef QT_NO_OPENSSL
    connect(_man, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(onIgnoreSSLErros(QNetworkReply*,QList<QSslError>)));
#endif
    connect(this, SIGNAL(doPost(QString,QMap<QString,QString>)), this, SLOT(onDoPost(QString,QMap<QString,QString>)));

    _postInProgress = false;
    _getInProgress = false;
}

HttpClient::~HttpClient()
{
    disconnect(_man, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));
    disconnect(_man, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(onIgnoreSSLErros(QNetworkReply*,QList<QSslError>)));
    disconnect(this, SIGNAL(doPost(QString,QMap<QString,QString>)), this, SLOT(onDoPost(QString,QMap<QString,QString>)));

    delete _man;
    delete _getMutex;
    delete _postMutex;
    delete _connectionLock;
}

void HttpClient::get(QString url)
{
    if(_isAborting)
        return;
    QMutexLocker l(_getMutex);
    Q_UNUSED(l);

    QUrl decodedUrl(url);
    int port = decodedUrl.port();

    if(port < 0 && decodedUrl.scheme().toLower() == "http")
        port = 80;
    else if(port < 0 && decodedUrl.scheme().toLower() == "https")
        port = 443;

    QString encodedUrl =decodedUrl.scheme() +"://"+ decodedUrl.host() + ":" + QString::number(port) + decodedUrl.path() +"?"+ Helper::getEncodedQueryString(decodedUrl, _connection);
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
        req.setRawHeader(first.toHtmlEscaped().toLatin1(), second.toHtmlEscaped().toLatin1());
#else
        req.setRawHeader(first.toAscii(), second.toAscii());
#endif
    }


    _connection->emitLogMessage("starting get request (" + _connection->getConnectionId() + "), active connections: " + QString::number(_currentConnections.size()+1), SignalR::Debug);
    _connection->emitLogMessage("get request url " +  QString::fromUtf8(reqUrl.toEncoded()), SignalR::Trace);
    QNetworkReply *getReply = _man->get(req);

#ifndef QT_NO_SSL
    getReply->setSslConfiguration(_connection->getSslConfiguration());
#endif

    _getInProgress = true;

    {
        QMutexLocker cl(_connectionLock);
        _currentConnections.append(getReply);
    }
}

void HttpClient::post(QString url, QMap<QString, QString> arguments)
{
    if(_isAborting)
        return;
    _postInProgress = true;
    Q_EMIT doPost(url, arguments);
}

QString HttpClient::postSync(QString url, QMap<QString, QString> arguments, QSharedPointer<SignalException> &error, int timeoutMs)
{
    Q_UNUSED(error);
    QString queryString;
    QMap<QString, QString>::iterator it = arguments.begin();
    while(it != arguments.end())
    {
        queryString.append(QString("%1=%2&").arg(it.key(), Helper::encode(it.value())));
        ++it;
    }

    queryString.remove(queryString.length()-1, 1);

    QUrl decodedUrl(url);
    int port = decodedUrl.port();

    if(port < 0 && decodedUrl.scheme().toLower() == "http")
        port = 80;
    else if(port < 0 && decodedUrl.scheme().toLower() == "https")
        port = 443;

    QString encodedUrl =decodedUrl.scheme() +"://"+ decodedUrl.host() + ":" + QString::number(port) + decodedUrl.path() +"?"+ Helper::getEncodedQueryString(decodedUrl, _connection);
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
        req.setRawHeader(first.toHtmlEscaped().toLatin1(), second.toHtmlEscaped().toLatin1());
#else
        req.setRawHeader(first.toAscii(), second.toAscii());
#endif
    }

    _connection->emitLogMessage("starting sync post request (" + _connection->getConnectionId() +")", SignalR::Debug);
    QNetworkAccessManager *networkMgr = new QNetworkAccessManager();
    QNetworkReply *postReply = networkMgr->post(req, QByteArray().append(queryString));

    QTimer t;
    t.setInterval(timeoutMs);
    t.start();

#ifndef QT_NO_SSL
    postReply->setSslConfiguration(_connection->getSslConfiguration());
#endif

    QEventLoop loop;

    QObject::connect(postReply, SIGNAL(finished()), &loop, SLOT(quit()));
    QObject::connect(&t, SIGNAL(timeout()), &loop, SLOT(quit()));

    loop.exec();

    QNetworkReply::NetworkError er = postReply->error();
    QString response = "";

    if(postReply->isFinished() && er == QNetworkReply::NoError) //post completed
    {
        response = QString(postReply->readAll());
    }
    else if(!postReply->isFinished()) //timed out
    {
        error = QSharedPointer<SignalException>(new SignalException(postReply->errorString(), SignalException::OperationCanceled));
    }
    else //other error
    {
        error = QSharedPointer<SignalException>(new SignalException(postReply->errorString(), SignalException::UnkownError));
    }

    delete postReply;
    delete networkMgr;

    _connection->emitLogMessage("sync post request finished(" + _connection->getConnectionId() +")", SignalR::Debug);
    return response;
}

void HttpClient::onDoPost(QString url, QMap<QString, QString> arguments)
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
    int port = decodedUrl.port();

    if(port < 0 && decodedUrl.scheme().toLower() == "http")
        port = 80;
    else if(port < 0 && decodedUrl.scheme().toLower() == "https")
        port = 443;

    QString encodedUrl =decodedUrl.scheme() +"://"+ decodedUrl.host() + ":" + QString::number(port) + decodedUrl.path() +"?"+ Helper::getEncodedQueryString(decodedUrl, _connection);
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
        req.setRawHeader(first.toHtmlEscaped().toLatin1(), second.toHtmlEscaped().toLatin1());
#else
        req.setRawHeader(first.toAscii(), second.toAscii());
#endif
    }

    _connection->emitLogMessage("starting post request (" + _connection->getConnectionId() +"), active connections: " + QString::number(_currentConnections.size()+1), SignalR::Debug);
    _connection->emitLogMessage("post request url %s" + QString::fromUtf8(reqUrl.toEncoded()), SignalR::Trace);

    QNetworkReply *postReply = _man->post(req, QByteArray().append(queryString));

#ifndef QT_NO_SSL
    postReply->setSslConfiguration(_connection->getSslConfiguration());
#endif

    _postInProgress = true;
    {
        QMutexLocker cl(_connectionLock);
        _currentConnections.append(postReply);
    }
}

void HttpClient::abort(bool dontEmitSignal)
{
    QMutexLocker l(_connectionLock);

    _isAborting = dontEmitSignal;

    foreach(QNetworkReply *reply, _currentConnections)
    {
        reply->abort();
    }
}

void HttpClient::abortPost()
{
    QMutexLocker l(_connectionLock);
    Q_UNUSED(l);

    foreach(QNetworkReply *reply, _currentConnections)
    {
        if(reply->operation() == QNetworkAccessManager::PostOperation)
            reply->abort();
    }
}

void HttpClient::abortGet()
{
    QMutexLocker l(_connectionLock);
    Q_UNUSED(l);

    foreach(QNetworkReply *reply, _currentConnections)
    {
        if(reply->operation() == QNetworkAccessManager::GetOperation)
            reply->abort();
    }
}

void HttpClient::requestFinished(QNetworkReply *reply)
{
    QNetworkReply::NetworkError error = reply->error();
    QNetworkAccessManager::Operation operation = reply->operation();

    if(error == QNetworkReply::OperationCanceledError && _isAborting)
    {
        {
            QMutexLocker l(_connectionLock);
            _currentConnections.removeOne(reply);
        }
        reply->deleteLater();
        _postInProgress = false;
        _getInProgress = false;
        return;
    }

    if(operation == QNetworkAccessManager::GetOperation)
    {
        _connection->emitLogMessage("get request finished", SignalR::Debug);
        _getInProgress = false;

        if(error == QNetworkReply::NoError)
            getRequestFinished(reply);
        else
            replyError(error, reply, "GET");
    }
    else if(operation == QNetworkAccessManager::PostOperation)
    {
        _postInProgress = false;
        _connection->emitLogMessage("post request finished", SignalR::Debug);

        if(error == QNetworkReply::NoError)
            postRequestFinished(reply);
        else
            replyError(error, reply, "POST");
    }
}

void HttpClient::getRequestFinished(QNetworkReply *reply)
{
    if(!reply)
        return;

    QString data = QString(reply->readAll());

    _connection->emitLogMessage(data, SignalR::Trace);

    QSharedPointer<SignalException> signalException = QSharedPointer<SignalException>(0);

    if(reply->error() == QNetworkReply::NoError)
    {
        {
            QMutexLocker l(_connectionLock);
            _currentConnections.removeOne(reply);
        }
        reply->deleteLater();
        Q_EMIT getRequestCompleted(data, signalException);
    }
}

void HttpClient::replyError(QNetworkReply::NetworkError err, QNetworkReply *reply, QString method)
{
    if(!reply)
        return;

    int error = (int)err;
    QString errorString = reply->errorString();

    _connection->emitLogMessage("request error " + method + " - " + errorString + " " + QString::number(error) + "Content: \n" + QString(reply->readAll()), SignalR::Error);

    if(error != QNetworkReply::NoError)
    {
        SignalException* ex = 0;


        switch(error)
        {
        case QNetworkReply::ConnectionRefusedError:
            ex = new SignalException(errorString, SignalException::ConnectionRefusedError);
            break;
        case QNetworkReply::RemoteHostClosedError:
            ex = new SignalException(errorString, SignalException::RemoteHostClosedConnection);
            break;
        case QNetworkReply::HostNotFoundError:
            ex = new SignalException(errorString, SignalException::HostNotFoundError);
            break;
        case QNetworkReply::TimeoutError:
            ex = new SignalException(errorString, SignalException::SocketOperationTimedOut);
            break;
        case QNetworkReply::OperationCanceledError:
            ex = new SignalException(errorString, SignalException::OperationCanceled);
            break;
        case QNetworkReply::SslHandshakeFailedError:
            ex = new SignalException(errorString, SignalException::SslHandshakeFailed);
            break;
        case QNetworkReply::TemporaryNetworkFailureError:
        case QNetworkReply::UnknownNetworkError:
            ex = new SignalException(errorString, SignalException::UnknownNetworkError);
            break;

        case QNetworkReply::ProxyConnectionRefusedError:
            ex = new SignalException(errorString, SignalException::ProxyConnectionRefusedError);
            break;
        case QNetworkReply::ProxyConnectionClosedError:
            ex = new SignalException(errorString, SignalException::ProxyConnectionClosedError);
            break;
        case QNetworkReply::ProxyNotFoundError:
            ex = new SignalException(errorString, SignalException::ProxyNotFoundError);
            break;
        case QNetworkReply::ProxyTimeoutError:
            ex = new SignalException(errorString, SignalException::ProxyTimeoutError);
            break;
        case QNetworkReply::ProxyAuthenticationRequiredError:
            ex = new SignalException(errorString, SignalException::ProxyAuthenticationRequiredError);
            break;
        case QNetworkReply::UnknownProxyError:
            ex = new SignalException(errorString, SignalException::UnknownProxyError);
            break;


        case QNetworkReply::ContentNotFoundError:
            ex = new SignalException(errorString, SignalException::ContentNotFoundError);
            break;
        case QNetworkReply::AuthenticationRequiredError:
            ex = new SignalException(errorString, SignalException::ServerRequiresAuthorization);
            break;
        case QNetworkReply::ContentReSendError:
        case QNetworkReply::ContentOperationNotPermittedError:
        case QNetworkReply::ContentAccessDenied:
        case QNetworkReply::UnknownContentError:
            ex = new SignalException(errorString, SignalException::UnkownContentError);
            break;

        case QNetworkReply::ProtocolUnknownError:
        case QNetworkReply::ProtocolInvalidOperationError:
        case QNetworkReply::ProtocolFailure:
            ex = new SignalException(errorString, SignalException::UnkownProtocolError);
            break;

        default:
            ex = new SignalException(errorString, SignalException::UnkownError);
            break;
        }
        QSharedPointer<SignalException> signalException = QSharedPointer<SignalException>(ex);
        if(!_isAborting)
        {
            {
                QMutexLocker l(_connectionLock);
                if(_currentConnections.contains(reply))
                    _currentConnections.removeOne(reply);
            }
            reply->deleteLater();

            if (reply->operation() == QNetworkAccessManager::GetOperation) {
                Q_EMIT getRequestCompleted("", signalException);
            } else {
                Q_EMIT postRequestCompleted("", signalException);
            }
        }
    }
}

void HttpClient::postRequestFinished(QNetworkReply *reply)
{
    if(!reply)
        return;

    QString data = QString(reply->readAll());

    _connection->emitLogMessage(data, SignalR::Trace);

    QSharedPointer<SignalException> signalException = QSharedPointer<SignalException>(0);

    if( reply->error() == QNetworkReply::NoError)
    {
        {
            QMutexLocker l(_connectionLock);
            _currentConnections.removeOne(reply);
        }
        reply->deleteLater();
        Q_EMIT postRequestCompleted(data, signalException);
    }
}

#ifndef QT_NO_SSL
void HttpClient::onIgnoreSSLErros(QNetworkReply *reply, QList<QSslError> error)
{
    if(!_connection->ignoreSslErrors())
        return;

    foreach(QSslError er, error)
    {
        _connection->emitLogMessage(er.errorString(), SignalR::Error);
    }

    reply->ignoreSslErrors(error);
}


#endif

}}}
