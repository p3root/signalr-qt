#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <QtCore>

class HttpResponse
{
public:
    HttpResponse();

    void addHeader(QString key, QString value) { _headers.insert(key, value); }
    void setContent(QString content)  { _content = content; }

    void setStatusCode(int status) { _statusCode = status; }
    const QByteArray getHttpResponse();

private:
    QString getStatusString();

private:
    QMap<QString, QString> _headers;
    QString _content;
    int _statusCode;
};

#endif // HTTPRESPONSE_H
