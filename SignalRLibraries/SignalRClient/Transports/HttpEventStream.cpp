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
    _sock = 0;

#ifdef Q_OS_WIN32
    QString host = QString(_url.host());
#else
    QString host = QString(_url.host());
#endif

    int port = _url.port();
    bool isSslSocket = false;

    //if no port is given in the url, set the default ports
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

    //create a new ssl socket if https is used
    if(_url.scheme() == "https")
    {
        QSslSocket *sslSocket = new QSslSocket();
        sslSocket->setSslConfiguration(_connection->getSslConfiguration());

        if(_connection->ignoreSslErrors())
            sslSocket->ignoreSslErrors();

        connect(sslSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslErrors(QList<QSslError>)), Qt::DirectConnection);

        _sock = sslSocket;
        isSslSocket = true;
    }
    else
    {
        //otherwise create a tcp socket
        _sock = new QTcpSocket();
    }

    QTextStream os(_sock);

    //setting the give proxy settings for the socket
    _sock->setProxy(_connection->getProxySettings());

    //try to resolve the hostname
    QHostInfo info = QHostInfo::fromName(host);

    if(info.error() == QHostInfo::NoError)
    {
        if(!info.addresses().isEmpty())
        {
            if(isSslSocket)
                ((QSslSocket*)_sock)->connectToHostEncrypted(_url.host(), port);
            else
                _sock->connectToHost(info.addresses().first(), port);
            if(_sock->waitForConnected())
            {
                _sock->setSocketOption(QAbstractSocket::KeepAliveOption,1);
                _sock->setSocketOption(QAbstractSocket::LowDelayOption, 1);

                QString getRequest = QString("%1 %2 %3").arg("GET", _url.path() +"?"+ Helper::getEncodedQueryString(_url, _connection), "HTTP/1.1\r\n");

                //prepare init http request
                os << QByteArray().append(getRequest);
                os << "Host: " << host << ":" << port << "\r\n";
                os << "User-Agent: SignalR.Client\r\n";
                os << "Accept: text/event-stream\r\n";

                //add additional http headers
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
                //write data to socket
                os.flush();

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
        //wait for the given connection timeout to get some data
        //if this functions returns false, no keep alive is give from the signalr server
        if(_sock->waitForReadyRead(_connection->getKeepAliveData().getTimeout()*1000))
        {
            if(!_sock->bytesAvailable())
                continue;

            if(!_isAborting && _sock->error() != 0)
            {
                if(_isFirstReponse)
                {
                    QString header = "";
                    //parse headers
                    while(!header.endsWith("\r\n\r\n"))
                    {
                        header += QString(_sock->read(1));
                    }

                    _isFirstReponse = false;
                }

                //parsing SSE package
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
            QAbstractSocket::SocketError socketError = _sock->error();
            QString errorString = _sock->errorString();

            if(!_isAborting)
            {

                SignalException *ex = 0;
                switch(socketError)
                {
                case QAbstractSocket::SocketTimeoutError:
                case QAbstractSocket::RemoteHostClosedError:
                    ex = new SignalException(errorString, SignalException::RemoteHostClosedConnection);
                    _isAborting = true;
                    break;
                case QAbstractSocket::SslHandshakeFailedError:
                    ex = new SignalException(errorString, SignalException::SslHandshakeFailed);
                    break;
                default:
                    ex = new SignalException(errorString, SignalException::EventStreamSocketLost);
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
    _sock->deleteLater();
    _sock = 0;
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

void HttpEventStream::onSslErrors(const QList<QSslError> &errors)
{
    if(!_connection->ignoreSslErrors())
        return;

    QSslSocket *socket = reinterpret_cast<QSslSocket*>(_sock);
    socket->ignoreSslErrors(errors);
}

}}}
