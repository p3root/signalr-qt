#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "SignalRHttp_global.h"
#include <QString>
#include <QMap>
#include <QUrl>

class SIGNALRHTTPSHARED_EXPORT HttpRequest
{
public:
    HttpRequest();

    static HttpRequest* parse(QString data);

    QString getHeader(QString key) { return _headers[key]; }
    QString getQueryString(QString key)  { return _query[key]; }
    QString getMethod() { return _method; }
    QUrl getUrl()  { return _url; }

protected:
    void addHeader(QString key, QString value) { _headers.insert(key, value); }
    void addQuery(QString key, QString value) { _query.insert(key, value); }
    void setMethod(QString method) { _method = method; }
    void setUrl(QUrl u) { _url = u; }

private:
    QMap<QString, QString> _headers;
    QMap<QString, QString> _query;
    QString _method;
    QUrl _url;

private:
    void setQuery(QString url);
};

#endif // HTTPREQUEST_H
