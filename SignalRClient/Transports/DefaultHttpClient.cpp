#include "DefaultHttpClient.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include "Helper/Helper.h"
#include <QsLog.h>
#include <unistd.h>

DefaultHttpClient::DefaultHttpClient()
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
    req.setRawHeader("User-Agent", "SignalR.Client");
    QNetworkReply *reply = nam->get(req);

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

    loop.exec();

    if(reply->error() == 0)
    {
        QString data = QString( reply->readAll());

        reply->deleteLater();
        reply = 0;

        delete nam;
        httpRequestCallback(data, 0, state);
    }
    else
    {
        httpRequestCallback("", new SignalException("", SignalException::EventStreamSocketLost), state);
        QLOG_WARN() << "http response not correctly finished " << reply->error() << " ... " << reply->errorString();
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

    QNetworkReply* reply = nam->post(req, QByteArray().append(queryString));

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));

    loop.exec();

    QByteArray data = reply->readAll();

    reply->deleteLater();
    reply = 0;

    delete nam;
    httpRequestCallback(QString(data), 0, state);
}
