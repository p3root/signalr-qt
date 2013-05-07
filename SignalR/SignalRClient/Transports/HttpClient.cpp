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
}

void HttpClient::get(QString url)
{
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
    /*QNetworkAccessManager* nam = new QNetworkAccessManager();

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

    QNetworkRequest req(reqUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setRawHeader("User-Agent", "SignalR-Qt.Client");

    QNetworkReply* reply = nam->post(req, QByteArray().append(queryString));
    _postReply = reply;
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));

    loop.exec();

    if(reply->error() == 0)
    {
        QByteArray data = reply->readAll();
        //httpRequestCallback(QString(data), 0, state);
    }
    else
    {
        //if(!_isAborting)
           // httpRequestCallback("", new SignalException(reply->errorString(), SignalException::CouldNotEstablishbConnection), state);
    }

    reply->deleteLater();
    reply = 0;
    _postReply = 0;
    nam->deleteLater();
    nam = 0;*/

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
