#ifndef DEFAULTHTTPCLIENT_H
#define DEFAULTHTTPCLIENT_H

#include "HttpClient.h"
#include <QNetworkAccessManager>

class DefaultHttpClient : public HttpClient
{
public:
    DefaultHttpClient();
    virtual ~DefaultHttpClient();

    void get(QString url, HTTP_REQUEST_CALLBACK httpRequestCallback,  void *state);
    void post(QString url, QMap<QString, QString> arguments, HTTP_REQUEST_CALLBACK httpRequestCallback, void *state);

    void abort();

private:
    bool _isAborting;
    QNetworkReply *_getReply;
    QNetworkReply *_postReply;
};

#endif // DEFAULTHTTPCLIENT_H
