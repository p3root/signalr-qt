#ifndef SERVERRESPONSE_H
#define SERVERRESPONSE_H

#include <QHttpResponse.h>

class ServerResponse
{
public:
    ServerResponse(QHttpResponse &res);

    QHttpResponse &getResponse() { return _response; }

    void write(const QString &data);
    void write(const QByteArray &data);

    void setContentType(QString contentType);
    const QString &getContentType() { return _contentType; }

    void closeResponse(int status);

private:
    QHttpResponse &_response;
    QString _contentType;
    QByteArray _data;
};

#endif // SERVERRESPONSE_H
