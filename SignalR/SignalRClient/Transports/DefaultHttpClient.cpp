#include "DefaultHttpClient.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include "Helper/Helper.h"
#include <QsLog.h>
#include <unistd.h>

DefaultHttpClient::DefaultHttpClient() : _isAborting(false), _getReply(0), _postReply(0)
{

}

DefaultHttpClient::~DefaultHttpClient()
{
}

void DefaultHttpClient::get(QString url, HTTP_REQUEST_CALLBACK httpRequestCallback, void *state)
{
    QNetworkAccessManager* nam = new QNetworkAccessManager();

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
    QNetworkReply *reply = nam->get(req);
    _getReply = reply;

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));

    loop.exec();

    QString data = QString(reply->readAll());
    int error = reply->error();
    QString errorString = reply->errorString();

    reply->deleteLater();
    reply = 0;
    _getReply = 0;
    nam->deleteLater();
    nam = 0;

    if(error == 0)
    {
        httpRequestCallback(data, 0, state);
    }
    else
    {
        SignalException* ex;
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
            httpRequestCallback("", ex, state);
    }
}

void DefaultHttpClient::post(QString url, QMap<QString, QString> arguments, HTTP_REQUEST_CALLBACK httpRequestCallback, void *state)
{
    QNetworkAccessManager* nam = new QNetworkAccessManager();

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
        httpRequestCallback(QString(data), 0, state);
    }
    else
    {
        if(!_isAborting)
            httpRequestCallback("", new SignalException(reply->errorString(), SignalException::CouldNotEstablishbConnection), state);
    }

    reply->deleteLater();
    reply = 0;
    _postReply = 0;
    nam->deleteLater();
    nam = 0;

}

void DefaultHttpClient::abort()
{
    _isAborting = true;

    if(_getReply)
        _getReply->abort();
    if(_postReply)
        _postReply->abort();
}
