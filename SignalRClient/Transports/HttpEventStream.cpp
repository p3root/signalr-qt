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
            /*QString test = QString(QByteArray(_sock->readAll()));
            QLOG_DEBUG() << test;
            readCallback(test, 0, state);
            return;*/


            if(_isFirstReponse)
            {
                //read http header
                QString header ="";

                while(!header.endsWith("\r\n\r\n"))
                {
                    header += QString(_sock->read(1));
                }

                _isFirstReponse = false;
            }
            QString packet = readPackage("");

            QLOG_DEBUG() << packet;
            readCallback(packet, 0, state);
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

QString HttpEventStream::readPackage(QString val)
{
    QString packetSize;
    while(!packetSize.endsWith("\r\n"))
    {
        packetSize += QString(_sock->read(1));
    }

    packetSize = packetSize.simplified();
    bool ok;
    int size = packetSize.toInt(&ok, 16);

    QString packetLine;
    while(packetLine.length() < size+2)
    {
        packetLine += QString(_sock->read(1));
    }

    val += packetLine;

    if(val.endsWith("\n\n\r\n"))
    {
        return val.simplified();
    }

    return readPackage(val);


}

