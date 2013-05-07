#include "HttpEventStream.h"
#include "Helper/Helper.h"
#include "Transports/ServerSentEventsTransport.h"
#include <QHostInfo>

HttpEventStream::HttpEventStream(QUrl url) : _sock(0), _isFirstReponse(true), _url(url)
{
    _isAborting = false;
}

void HttpEventStream::open()
{
    _sock = new QTcpSocket();
    QTextStream os(_sock);

#ifdef Q_OS_WIN32
    QString host = QString(_url.host());
#else
    QString host = QString(_url.encodedHost());
#endif

    QHostInfo info = QHostInfo::fromName(host);

    if(info.error() == QHostInfo::NoError)
    {
        if(!info.addresses().isEmpty())
        {
            _sock->connectToHost(info.addresses().first(), _url.port());
            if(_sock->waitForConnected())
            {
                _sock->setSocketOption(QAbstractSocket::KeepAliveOption,1);

                QString getRequest = QString("%1 %2 %3").arg("GET", _url.path() +"?"+ Helper::getEncodedQueryString(_url), "HTTP/1.1\r\n");

                //prepare http request
                os << QByteArray().append(getRequest);
                os << "Host: " << host << ":" << _url.port() << "\r\n";
                os << "User-Agent: SignalR.Client\r\n";
                os << "Accept: text/event-stream\r\n";
                os << "\r\n";
                os.flush();

                connect(_sock, SIGNAL(readyRead()), this, SLOT(onReadyRead()), Qt::QueuedConnection);

                Q_EMIT connected(0);
            }
            else
            {
                Q_EMIT connected(new SignalException(_sock->errorString(), SignalException::ConnectionRefusedError));
                QLOG_ERROR() << "EventStream Error:" << _sock->errorString();
            }
        }
    }
    else
    {
        Q_EMIT connected(new SignalException(info.errorString(), SignalException::ConnectionRefusedError));
        QLOG_ERROR() << "Host Error:" << info.errorString();
    }

}

void HttpEventStream::close()
{
    _isAborting = true;
    _sock->abort();

    QEventLoop loop;
    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));

    loop.exec();
}

void HttpEventStream::run()
{
    open();

    while(!_isAborting)
    {
        if(_sock->waitForReadyRead(-1))
        {
            if(!_isAborting && _sock->error() != 0)
            {
                if(_isFirstReponse)
                {
                    QString header = "";
                    while(!header.endsWith("\r\n\r\n"))
                    {
                        header += QString(_sock->read(1));
                    }

                    _isFirstReponse = false;
                }

                QString pack = readPackage("");
                Q_EMIT packetReady(pack, 0);
            }
            else
            {
                if(!_isAborting)
                {
                    Q_EMIT packetReady("", new SignalException("", SignalException::EventStreamSocketLost));
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            if(!_isAborting)
            {
                Q_EMIT packetReady("", new SignalException("", SignalException::EventStreamSocketLost));
            }
            else
            {
                break;
            }
        }
    }

    delete _sock;
}

QString HttpEventStream::readPackage(QString val)
{
    QString packetSize;
    int count = 0;
    while(!packetSize.endsWith("\r\n"))
    {
        if(_sock->bytesAvailable() == 0)
            _sock->waitForReadyRead(100);
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

void HttpEventStream::onReadyRead()
{


}
