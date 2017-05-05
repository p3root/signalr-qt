/*
 *  Copyright (c) 2013-2014, p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at)
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
#include "Connection_p.h"
#include "HttpEventStreamParser.h"
#include <QUuid>

#if defined(Q_OS_QNX)
#include <QtNetwork/qhostinfo.h>
#else
#include <QHostInfo>
#endif

namespace P3 { namespace SignalR { namespace Client {

#define PACKAGE_FINISHED "\r\n"

HttpEventStream::HttpEventStream(QUrl url, ConnectionPrivate *con, QObject *parent) : QObject(parent), _sock(0), _url(url)
{
    _connection = con;
    _isRunning = false;
    _parser = new HttpEventStreamParser();
    _connected = false;
    _id = QUuid::createUuid().toString();
}

HttpEventStream::~HttpEventStream()
{
    _isAborting = true;

    if(_sock)
    {
        disconnect(_sock, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        disconnect(_sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));

        if(_sock->isOpen())
            _sock->close();
        _sock->deleteLater();
        _sock = 0;
    }

    delete _parser;
}

void HttpEventStream::open()
{
    if(_isRunning)
        return;

    QSharedPointer<SignalException> error;

    _isRunning = true;

    _isAborting = false;
    _parser->reset();

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
#ifndef QT_NO_SSL
        QSslSocket *sslSocket = new QSslSocket();
        sslSocket->setSslConfiguration(_connection->getSslConfiguration());

        if(_connection->ignoreSslErrors())
            sslSocket->ignoreSslErrors();

        connect(sslSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslErrors(QList<QSslError>)), Qt::DirectConnection);

        _sock = sslSocket;
        isSslSocket = true;
#else
        _sock = new QTcpSocket();
#endif
    }
    else
    {
        //otherwise create a tcp socket
        _sock = new QTcpSocket();
    }

    QTextStream os(_sock);

#ifndef QT_NO_NETWORKPROXY
    //setting the give proxy settings for the socket
    _sock->setProxy(_connection->getProxySettings());
#endif

    connect(_sock, SIGNAL(readyRead()), this, SLOT(onReadyRead()));


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

            if(!_sock)
                return;
            if(_sock->waitForConnected())
            {
                connect(_sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
                _sock->setSocketOption(QAbstractSocket::KeepAliveOption,1);
                _sock->setSocketOption(QAbstractSocket::LowDelayOption, 1);

                QString getRequest = QString("%1 %2 %3").arg("GET", _url.path() +"?"+ Helper::getEncodedQueryString(_url, _connection), "HTTP/1.1\r\n");

                //prepare init http request
                os << getRequest;
                os << "Host: " << host << ":" << port << "\r\n";
                os << "User-Agent: SignalR-Qt.Client\r\n";
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

                _connection->emitLogMessage(_id + " -- " + getRequest, SignalR::Trace);

                os << "\r\n";
                //write data to socket
                os.flush();
            }
            else
            {
                QString errorString = _sock->errorString();

                error = QSharedPointer<SignalException>(new SignalException(errorString, SignalException::ConnectionRefusedError));
                Q_EMIT connected(error);
            }
        }
    }
    else
    {
        QString errorString = info.errorString();
        error = QSharedPointer<SignalException>(new SignalException(errorString, SignalException::ConnectionRefusedError));
        Q_EMIT connected(error);
    }

}

void HttpEventStream::close()
{
    if(_sock)
    {
        _sock->abort();
        _sock->close();
        _sock->deleteLater();
        _sock = 0;

        onSocketError(QAbstractSocket::NetworkError);
    }
}

#ifndef QT_NO_SSL
void HttpEventStream::onSslErrors(const QList<QSslError> &errors)
{
    if(!_connection->ignoreSslErrors())
        return;

    QSslSocket *socket = reinterpret_cast<QSslSocket*>(_sock);
    socket->ignoreSslErrors(errors);
}
#endif

void HttpEventStream::onReadyRead()
{
    _parser->addData(_sock->readAll());

    HttpEventStreamParserResult result;
    while(_parser->next(result))
    {
        QString data = QString::fromUtf8(result.packet);

        _connection->emitLogMessage("SSE Received: " + data + " (Status " + (!result.error.isNull() ? result.error->getMessage() : "Ok")+")", SignalR::Trace);

        if(data == "data: initialized") {
            _connected = true;
            _connection->changeState(_connection->getState(), SignalR::Connected);
            Q_EMIT connected(result.error);
        }
        else if(data == "The ConnectionId is in the incorrect format.") {
            Q_EMIT restartConnection();
        }
        else if(!_connected) {
            Q_EMIT connected(result.error);
        }
        else {
            int dataEndIndex = data.indexOf("\r\n\r\n");

            if(data.length() > dataEndIndex) {
                do {
                    QString package = data.mid(0, dataEndIndex);
                    Q_EMIT packetReady(package, result.error);

                    data = data.mid(dataEndIndex+4, data.length()-dataEndIndex);
                    dataEndIndex = data.indexOf("\r\n\r\n");
                }
                while(dataEndIndex > 0);
            }
            else
                Q_EMIT packetReady(data, result.error);
        }

        if(!result.error.isNull())
        {
            disconnect(_sock, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

            if(_sock->isOpen())
                _sock->close();
            _sock->deleteLater();
            _sock = 0;
        }
    }
}

void HttpEventStream::onSocketError(QAbstractSocket::SocketError error)
{
    QAbstractSocket::SocketError socketError = error;
    QString errorString = _sock ? _sock->errorString() : "";

    if(!_isAborting)
    {

        QSharedPointer<SignalException> ex;
        switch(socketError)
        {
        case QAbstractSocket::SocketTimeoutError:
        case QAbstractSocket::RemoteHostClosedError:
            ex = QSharedPointer<SignalException>(new SignalException(errorString, SignalException::RemoteHostClosedConnection));
            //_isAborting = true;
            break;
        case QAbstractSocket::SslHandshakeFailedError:
            ex = QSharedPointer<SignalException>(new SignalException(errorString, SignalException::SslHandshakeFailed));
            break;
        case QAbstractSocket::NetworkError:
            ex = QSharedPointer<SignalException>(new SignalException("Operation canceled", SignalException::OperationCanceled));
            break;
        default:
            ex = QSharedPointer<SignalException>(new SignalException(errorString, SignalException::UnknownNetworkError));
            break;

        }

        if(_connected)
            Q_EMIT packetReady("", ex);
        else
            Q_EMIT connected(ex);
    }
}

}}}
