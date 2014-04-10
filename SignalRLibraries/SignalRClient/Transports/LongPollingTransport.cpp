/*
 *  Copyright (c) 2013-2014, p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at)
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *
 *    Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright notice, this
 *    list of conditions and the following disclaimer in the documentation and/or
 *    other materials provided with the distribution.
 *
 *    Neither the name of the {organization} nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "LongPollingTransport.h"
#include "Helper/Helper.h"

namespace P3 { namespace SignalR { namespace Client {

LongPollingTransport::LongPollingTransport(HttpClient* httpClient, Connection *con) :HttpBasedTransport(httpClient, con)
{
    _started = false;
    _lastSignalException = 0;

    connect(&_keepAliveTimer, SIGNAL(timeout()), this, SLOT(keepAliveTimerTimeout()));
}


LongPollingTransport::~LongPollingTransport(void)
{
}

void LongPollingTransport::start(QString)
{
    connect(_httpClient, SIGNAL(getRequestCompleted(QString,SignalException*)), this, SLOT(onPollHttpResponse(QString,SignalException*)));

    _keepAliveTimer.setInterval(120*1000); //default on the signalr server is 110 sec, just to ensure not to run in a socket timeout
    _keepAliveTimer.setSingleShot(true);

    _connection->updateLastKeepAlive();

    QString conOrRecon = "connect";
    if(_started)
        conOrRecon = "reconnect";
    QString connectUrl = _connection->getUrl() + "/" +conOrRecon;
    connectUrl += TransportHelper::getReceiveQueryString(_connection, _connection->onSending(), getTransportType());

    connect(_httpClient, SIGNAL(postRequestCompleted(QString,SignalException*)), SLOT(onPostRequestCompleted(QString,SignalException*)));
    _httpClient->post(connectUrl, QMap<QString, QString>());
}

void LongPollingTransport::abort()
{
    _httpClient->abort();
}

void LongPollingTransport::stop()
{
    _httpClient->abort();
}

void LongPollingTransport::retry()
{
    HttpBasedTransport::retry();

    if(_retryTimerTimeout.isActive())
    {
        startConnection();
    }
}

const QString &LongPollingTransport::getTransportType()
{
    static QString type  ="longPolling";
    return type;
}

void LongPollingTransport::startConnection()
{
    _url = _connection->getUrl() + "/poll";
    _url += TransportHelper::getReceiveQueryString(_connection, _connection->onSending(), getTransportType());
    connect(_httpClient, SIGNAL(getRequestCompleted(QString,SignalException*)), this, SLOT(onPollHttpResponse(QString,SignalException*)));
    _httpClient->get(_url);
    _keepAliveTimer.start();
}

void LongPollingTransport::onPollHttpResponse(const QString& httpResponse, SignalException *ex)
{
    _keepAliveTimer.stop();
    _lastSignalException = ex;

    disconnect(_httpClient, SIGNAL(getRequestCompleted(QString,SignalException*)), this, SLOT(onPollHttpResponse(QString,SignalException*)));
    bool timedOut = false, disconnected = false, serverError = false;
    SignalException *error = 0;
    if(ex)
        error = (SignalException*)ex;

    if(!error)
    {
        _connection->updateLastKeepAlive();
        TransportHelper::processMessages(_connection, httpResponse, &timedOut, &disconnected);
        if(_connection->getState() != Connection::Connected)
            _connection->changeState(_connection->getState(), Connection::Connected);
    }
    else
    {
        if(error->getType() == SignalException::ServerRequiresAuthorization)
        {
            _connection->onError(*error);
            disconnected = true;
        }
        else
        {
            serverError = true;

            if(_started)
            {
                 _connection->onError(*error);

                if(error->getType() == SignalException::ContentNotFoundError
                        || error->getType() == SignalException::ConnectionRefusedError
                        || error->getType() == SignalException::InternalServerError) {

                    //_started = false;
                    _connection->changeState(Connection::Connected, Connection::Reconnecting);

                    _connection->emitLogMessage("LP: Lost connection, trying to reconnect in " + QString::number(_connection->getReconnectWaitTime()) + "s", Connection::Debug);

                    connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectErrorRetry()));
                    _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime()*1000);
                    _retryTimerTimeout.start();

                    return;

                }
                else if(error->getType() == SignalException::OperationCanceled)
                {
                    serverError = false;
                    _connection->emitLogMessage("LP: Connection was closed due to a client timeout", Connection::Warning);

                    connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(errorRetryTimer()));
                    _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime()*1000);
                    _retryTimerTimeout.start();
                    _connection->updateLastKeepAlive();

                    return;
                }
                else if(error->getType() == SignalException::UnknownNetworkError)
                {
                    serverError = false;
                    _connection->emitLogMessage("LP: Connection was closed due to an unknown network error", Connection::Error);
                    _connection->changeState(_connection->getState(), Connection::Reconnecting);
                    connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(errorRetryTimer()));
                    _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime()*1000);
                    _retryTimerTimeout.start();

                    return;
                }
                else
                {
                    _connection->emitLogMessage("LP: Unknown error (" + error->getMessage() + ")", Connection::Error);

                    connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectErrorRetry()));
                    _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime()*1000);
                    _retryTimerTimeout.start();

                    return;
                }

                if(serverError)
                {
                    Q_EMIT transportStarted(ex);
                    _keepAliveTimer.start();
                }

            }
            else
            {

                Connection::State state = Connection::Disconnected;
                _connection->updateLastKeepAlive();

                if(!_connection->getConnectionId().isEmpty())
                    state = Connection::Disconnected;

                if(error->getType() == SignalException::ConnectionRefusedError ||
                        error->getType() == SignalException::ServerRequiresAuthorization)
                {
                    _connection->changeState(_connection->getState(), state);
                }
                else
                {
                    _connection->emitLogMessage("LP: Unkown error (" + error->getMessage() + ")", Connection::Error);
                }

                if(_connection->ensureReconnecting())
                {
                     _connection->emitLogMessage("LP: Lost connection, try to reconnect in " + QString::number(_connection->getReconnectWaitTime()) + "s", Connection::Debug);

                    connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectErrorRetry()));
                    _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime()*1000);
                    _retryTimerTimeout.start();

                    return;
                }
                else if(_connection->getAutoReconnect())
                {
                    _connection->emitLogMessage("LP: Lost connection, try to reconnect in " + QString::number(_connection->getReconnectWaitTime()) + "s", Connection::Debug);

                    connect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectErrorRetry()));
                    _retryTimerTimeout.setInterval(_connection->getReconnectWaitTime()*1000);
                    _retryTimerTimeout.start();

                    return;
                }
                else
                {
                    _connection->onError(*error);
                }
            }
        }

        delete error;
        error = 0;
    }

    if(disconnected)
    {
        _connection->stop();
    }
    else if(!serverError && !_connection->getConnectionToken().isEmpty())
    {
       startConnection();
    }

}

void LongPollingTransport::onPostRequestCompleted(const QString &httpResponse, SignalException *error)
{
    Q_UNUSED(httpResponse);

    disconnect(_httpClient, SIGNAL(postRequestCompleted(QString,SignalException*)),this, SLOT(onPostRequestCompleted(QString,SignalException*)));

    if(!error && !_connection->getConnectionToken().isEmpty())
    {
        _url = _connection->getUrl() + "/poll";
        _url += TransportHelper::getReceiveQueryString(_connection, _connection->onSending(), getTransportType());

        _started = true;
        _httpClient->get(_url);

        Q_EMIT transportStarted(0);
    }
    else
    {
        Q_EMIT transportStarted(error);
    }
}

void LongPollingTransport::keepAliveTimerTimeout()
{
    _httpClient->abort(false);
}

void LongPollingTransport::errorRetryTimer()
{
    _retryTimerTimeout.stop();
    disconnect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(errorRetryTimer()));
    disconnect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectErrorRetry()));
    startConnection();
}

void LongPollingTransport::reconnectErrorRetry()
{
    _retryTimerTimeout.stop();
    disconnect(&_retryTimerTimeout, SIGNAL(timeout()), this, SLOT(reconnectErrorRetry()));

    _keepAliveTimer.start();
    Q_EMIT transportStarted(_lastSignalException);

}

}}}
