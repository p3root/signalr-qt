#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QNetworkAccessManager>
#include "SignalException.h"
#include <QMap>
#include <QNetworkReply>

class HttpClient : public QObject
{
    Q_OBJECT
public:
    HttpClient();
    virtual ~HttpClient();

    void get(QString url);
    void post(QString url, QMap<QString, QString> arguments);

    void abort();

Q_SIGNALS:
    void getRequestCompleted(const QString& httpResponse, SignalException* error);
    void postRequestCompleted(const QString& httpResponse, SignalException* error);

private Q_SLOTS:
    void getRequestFinished();
    void getError(QNetworkReply::NetworkError);

private:
    bool _isAborting;
    QNetworkReply *_getReply;
    QNetworkReply *_postReply;
    QNetworkAccessManager* _man;
};

#endif // HTTPCLIENT_H
