#include "HttpEventStream.h"
#include "Helper/Helper.h"

HttpEventStream::HttpEventStream() : _sock(0), _isFirstReponse(true), _isAborting(false)
{

}

void HttpEventStream::get(QUrl url, HTTP_EVENT_REQUEST_CALLBACK callback, void* state)
{

    _sock = new QTcpSocket();
    QTextStream os(_sock);

#ifdef Q_OS_WIN32
    QString host = QString(url.host());
#else
    QString host = QString(url.encodedHost());
#endif


    _sock->connectToHost(QHostAddress(host), url.port());
    if(_sock->waitForConnected())
    {
        _sock->setSocketOption(QAbstractSocket::KeepAliveOption,1);

        QString getRequest = QString("%1 %2 %3").arg("GET", url.path() +"?"+ Helper::getEncodedQueryString(url), "HTTP/1.1\r\n");
        QLOG_DEBUG() << getRequest;

        os << QByteArray().append(getRequest);
        os << "Host: " << host << ":" << url.port() << "\r\n";
        os << "User-Agent: SignalR.Client\r\n";
        os << "Accept: text/event-stream\r\n";
        os << "\r\n";
        os.flush();

        callback(*this, NULL, state);
    }
    else
    {
        QLOG_DEBUG() << _sock->errorString();
    }

}

void HttpEventStream::abort()
{
    _isAborting = true;
    QLOG_DEBUG() << "HttpEventStream is closing...";
}


void HttpEventStream::readLine(HttpResponse::READ_CALLBACK readCallback, void *state)
{
    if(_sock == 0)
        return;

    if(_sock->waitForReadyRead(-1))
    {
        if(!_isAborting && _sock->error() != 0)
        {
            //TODO READ correct!!
            QString response = QString(QByteArray(_sock->readAll()));
            if(_isFirstReponse)
            {
                //remove http header infos
                response = response.remove(0, response.indexOf("\r\n\r\n")+4);
                response = response.remove(0, response.indexOf("\r\n"));
                response = response.replace("\n", "");
                response = response.replace("\r", "");

                _isFirstReponse = false;
            }
            QLOG_DEBUG() << response;
            readCallback(response, 0, state);
        }
        else
        {
            if(_isAborting)
            {
                _sock->abort();
                _sock->deleteLater();
                return;
            }
            readCallback("", new SignalException("", SignalException::EventStreamSocketLost), state);
        }
    }
}

