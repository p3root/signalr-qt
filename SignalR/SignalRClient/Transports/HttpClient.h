#ifndef IHTTPCLIENT_H
#define IHTTPCLIENT_H

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "SignalException.h"
#include <QString>
#include <QMap>
#include <QObject>


class HttpClient : public QObject
{
    Q_OBJECT
public:
    HttpClient(void);
    virtual	~HttpClient(void);

    typedef void (*HTTP_REQUEST_CALLBACK)(const QString& httpResponse, SignalException* error, void* state);

    virtual void get(QString url, HTTP_REQUEST_CALLBACK httpRequestCallback, void* state = 0) = 0;
    virtual void post(QString url, QMap<QString, QString> arguments, HTTP_REQUEST_CALLBACK httpRequestCallback, void* state = 0) = 0;

    virtual void abort()=0;
};

#endif
