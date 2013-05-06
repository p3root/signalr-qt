#include "HttpEventStream.h"
#include "Helper/Helper.h"
#include "Transports/ServerSentEventsTransport.h"

HttpEventStream::HttpEventStream() : _sock(0), _isFirstReponse(true)
{
    _isAborting = false;
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

        //prepare http request
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
        callback(*this, new SignalException(_sock->errorString(), SignalException::ConnectionRefusedError), state);
        QLOG_ERROR() << "EventStream Error:" << _sock->errorString();
    }

}

void HttpEventStream::abort()
{

}

void HttpEventStream::closeConnection()
{
    _isAborting = true;
}


void HttpEventStream::readLine(HttpResponse::READ_CALLBACK readCallback, void *state)
{
    if(_sock == 0)
        return;
    //TODO: error implement handling
    if(_sock->waitForReadyRead(-1))
    {
        if(!_isAborting && _sock->error() != 0)
        {
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
    else
    {
        _sock->close();
        _sock->deleteLater();
        readCallback("", new SignalException("", SignalException::EventStreamSocketLost), state);
    }
}

QString HttpEventStream::readPackage(QString val)
{
    QString packetSize;
    int count = 0;
    while(!packetSize.endsWith("\r\n"))
    {
        if(packetSize.isEmpty() && count >= 10)
            return "";
        packetSize += QString(_sock->read(1));
        count++;
    }

    packetSize = packetSize.simplified();
    bool ok;
    int size = packetSize.toInt(&ok, 16);

    if(!ok)
        return "";

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


