#ifndef HTTPEVENTSTREAM_H
#define HTTPEVENTSTREAM_H

#include <QtCore>

#if defined(Q_OS_QNX)
	#include <QtNetwork/qhostaddress.h>
	#include <QtNetwork/qtcpsocket.h>
#else
	#include <QHostAddress>
	#include <QTcpSocket>
#endif
#include <QsLog.h>
#include "HttpRequest.h"
#include "HttpResponse.h"

class ServerSentEventsTransport;

class HttpEventStream : public QThread
{
    Q_OBJECT

public:
    HttpEventStream(QUrl url);

    void close();

    void run();

Q_SIGNALS:
    void packetReady(QString packet, SignalException *ex);
    void connected(SignalException*);

private:
    void open();

private Q_SLOTS:
    void onReadyRead();

private:
    QTcpSocket *_sock;
    bool _isFirstReponse;
    bool _isAborting;
    QMutex _mutex;
    QUrl _url;

    QString readPackage(QString);

};

#endif // EVENTSTREAM_H
