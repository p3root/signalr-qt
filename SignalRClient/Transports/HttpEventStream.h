#ifndef HTTPEVENTSTREAM_H
#define HTTPEVENTSTREAM_H

#include <QtCore>
#include <QHostAddress>
#include <QTcpSocket>
#include <QsLog.h>
#include "HttpRequest.h"
#include "HttpResponse.h"

class HttpEventStream : public QObject, public HttpResponse
{
    Q_OBJECT

public:
    HttpEventStream();

    typedef void (*HTTP_EVENT_REQUEST_CALLBACK)(HttpResponse& httpResponse, SignalException* error, void* state);

    void get(QUrl url, HTTP_EVENT_REQUEST_CALLBACK callback, void* state);
    void abort();

    void readLine(READ_CALLBACK readCallback, void* state = 0);
private:
    QTcpSocket *_sock ;
    bool _isFirstReponse;
    bool _isAborting;

};

#endif // EVENTSTREAM_H
