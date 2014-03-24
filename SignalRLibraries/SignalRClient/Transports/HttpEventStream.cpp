/*
 *  Copyright (c) 2013, p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at)
 *  All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. All advertising materials mentioning features or use of this software
 *      must display the following acknowledgement:
 *      This product includes software developed by the p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at).
 *   4. Neither the name of the p3root - Patrik Pfaffenbauer nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY p3root - Patrik Pfaffenbauer ''AS IS'' AND ANY
 *   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *   DISCLAIMED. IN NO EVENT SHALL p3root - Patrik Pfaffenbauer BE LIABLE FOR ANY
 *   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "HttpEventStream.h"
#include "Helper/Helper.h"
#include "Transports/ServerSentEventsTransport.h"

#if defined(Q_OS_QNX)
#include <QtNetwork/qhostinfo.h>
#else
#include <QHostInfo>
#endif

namespace P3 { namespace SignalR { namespace Client {

HttpEventStream::HttpEventStream(QUrl url, bool logErrorsToQt, Connection *con) : _sock(0), _isFirstReponse(true), _url(url)
{
    _logErrorsToQt = logErrorsToQt;
    _connection = con;
}

void HttpEventStream::open()
{
    _isAborting = false;
    _isFirstReponse = true;
    _sock = new QTcpSocket();
    QTextStream os(_sock);

#ifdef Q_OS_WIN32
    QString host = QString(_url.host());
#else
    QString host = QString(_url.host());
#endif

    int port = _url.port();

    if(port < 0)
    {
        if(_url.scheme() == "http")
        {
            port = 80;
        }
        else if(_url.scheme() == "https")
        {
            port = 443;
        }
    }

    QHostInfo info = QHostInfo::fromName(host);

    if(info.error() == QHostInfo::NoError)
    {
        if(!info.addresses().isEmpty())
        {
            _sock->connectToHost(info.addresses().first(), port);
            if(_sock->waitForConnected())
            {
                _sock->setSocketOption(QAbstractSocket::KeepAliveOption,1);

                QString getRequest = QString("%1 %2 %3").arg("GET", _url.path() +"?"+ Helper::getEncodedQueryString(_url, _connection), "HTTP/1.1\r\n");

                //prepare http request
                os << QByteArray().append(getRequest);
                os << "Host: " << host << ":" << port << "\r\n";
                os << "User-Agent: SignalR.Client\r\n";
                os << "Accept: text/event-stream\r\n";

                for(int i = 0; i < _connection->getAdditionalHttpHeaders().size(); i++)
                {
                    QString first = QString(_connection->getAdditionalHttpHeaders().at(i).first);
                    QString second = QString(_connection->getAdditionalHttpHeaders().at(i).second);
                    os << first;
                    os << ": ";
                    os << second;
                    os << "\r\n";
                }

                os << "\r\n";
                os.flush();

                connect(_sock, SIGNAL(readyRead()), this, SLOT(onReadyRead()), Qt::QueuedConnection);

                Q_EMIT connected(0);
            }
            else
            {
                Q_EMIT connected(new SignalException(_sock->errorString(), SignalException::ConnectionRefusedError));
                if(_logErrorsToQt)
                    qCritical() << "EventStream Error:" << _sock->errorString();
            }
        }
    }
    else
    {
        Q_EMIT connected(new SignalException(info.errorString(), SignalException::ConnectionRefusedError));
        if(_logErrorsToQt)
            qCritical() << "Host Error:" << info.errorString();
    }

}

void HttpEventStream::close()
{
    _isAborting = true;
}

void HttpEventStream::run()
{
    open();

    while(!_isAborting)
    {
        if(_sock->waitForReadyRead(_connection->getKeepAliveData().getTimeout()*1000))
        {
            if(!_sock->bytesAvailable())
                continue;

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
                if(!pack.isEmpty())
                {
                    Q_EMIT packetReady(pack, 0);
                }
            }
            else
            {
                if(!_isAborting)
                {
                    Q_EMIT packetReady("", new SignalException(_sock->errorString(), SignalException::EventStreamSocketLost));
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
                QAbstractSocket::SocketError socketError = _sock->error();
                SignalException *ex = 0;
                switch(socketError)
                {
                    case QAbstractSocket::SocketTimeoutError:
                    case QAbstractSocket::RemoteHostClosedError:
                        ex = new SignalException(_sock->errorString(), SignalException::RemoteHostClosedConnection);
                        _isAborting = true;
                        break;
                    default:
                        ex = new SignalException(_sock->errorString(), SignalException::EventStreamSocketLost);
                        break;

                }

                Q_EMIT packetReady("", ex);
            }
            else
            {
                break;
            }
        }
    }

    if(_sock->isOpen())
        _sock->close();
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

}}}
