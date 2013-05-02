#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "SignalRHttp_global.h"
#include <QString>
#include <QMap>

class SIGNALRHTTPSHARED_EXPORT HttpRequest
{
public:
    HttpRequest();

    static HttpRequest* parse(QString data);

    QString getHeader(QString key) { return _headers[key]; }
    QString getContent() { return _content; }

protected:
    void addHeader(QString key, QString value) { _headers.insert(key, value); }
    void addContent(QString content) { _content = content; }

private:
    QMap<QString, QString> _headers;
    QString _content;
};

#endif // HTTPREQUEST_H
