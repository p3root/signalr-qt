#include "HttpClient.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include "Helper/Helper.h"
#include <QsLog.h>
#include <unistd.h>

HttpClient::HttpClient() : _isAborting(false), _getReply(0), _postReply(0), _man(0)
{
    _man = new QNetworkAccessManager();
}

HttpClient::~HttpClient()
{
    delete _man;
}

void HttpClient::get(QString url)
{
    if(_getReply)
    {
        delete _getReply;
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
    if(_postReply)
    {
        delete _postReply;
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
