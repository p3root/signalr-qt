/*
 *  Copyright (c) 2013, p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at)
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

LongPollingTransport::LongPollingTransport(HttpClient* httpClient, Connection *con) :HttpBasedTransport(httpClient, con)
{
    _started = false;

    connect(&_keepAliveTimer, SIGNAL(timeout()), this, SLOT(keepAliveTimerTimeout()));
}


LongPollingTransport::~LongPollingTransport(void)
{
}

void LongPollingTransport::start(QString)
{
    connect(_httpClient, SIGNAL(getRequestCompleted(QString,SignalException*)), this, SLOT(onPollHttpResponse(QString,SignalException*)));

    _keepAliveTimer.setInterval(_connection->getKeepAliveData().getConnectionTimeout()*1000); //default on the signalr server is 110 sec, just to ensure not to run in a socket timeout
    _keepAliveTimer.start();

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

const QString &LongPollingTransport::getTransportType()
{
    static QString type  ="longPolling";
    return type;
}

void LongPollingTransport::onPollHttpResponse(const QString& httpResponse, SignalException *ex)
{
    if(_keepAliveTimer.isActive())
        _keepAliveTimer.stop();
    disconnect(_httpClient, SIGNAL(getRequestCompleted(QString,SignalException*)), this, SLOT(onPollHttpResponse(QString,SignalException*)));
    bool timedOut = false, disconnected = false, serverError = false;
    SignalException *error = 0;
    if(ex)
        error = (SignalException*)ex;

    if(!error)
    {
        TransportHelper::processMessages(_connection, httpResponse, &timedOut, &disconnected);
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
                if(error->getType() == SignalException::ContentNotFoundError
                || error->getType() == SignalException::ConnectionRefusedError) {

                    _started = false;
                    _connection->changeState(Connection::Connected, Connection::Disconnected);

                    if(_connection->getLogErrorsToQDebug())
                        qDebug() << "LongPollingTranpsort: lost connection...try to reconnect";

                    _connection->emitLogMessage("lost connection...try to reconnect", Connection::Debug);
                    Helper::wait(_connection->getReconnectWaitTime());
                }
                else if(error->getType() == SignalException::OperationCanceled || error->getType() == SignalException::UnkownNetworkError)
                {
                    serverError = false;
                    _connection->emitLogMessage("connection was closed due a client timeout", Connection::Warning);
                }
                else
                {
                    _connection->emitLogMessage("unkown error (" + error->getMessage() + ")", Connection::Error);
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
                    _connection->emitLogMessage("unkown error (" + error->getMessage() + ")", Connection::Error);
                }

                if(_connection->ensureReconnecting())
                {
                    if(_connection->getLogErrorsToQDebug())
                        qDebug() << "LongPollingTranpsort: lost connection...try to reconnect";

                    _connection->emitLogMessage("lost connection...try to reconnect", Connection::Debug);

                    Helper::wait(_connection->getReconnectWaitTime());
                    _keepAliveTimer.start();
                    Q_EMIT transportStarted(ex);
                }
                else if(_connection->getAutoReconnect())
                {
                    if(_connection->getLogErrorsToQDebug())
                        qDebug() << "LongPollingTranpsort: (autoconnect=true) lost connection...try to reconnect";
                    _connection->emitLogMessage("lost connection...try to reconnect", Connection::Debug);

                    Helper::wait(_connection->getReconnectWaitTime());
                    _keepAliveTimer.start();
                    Q_EMIT transportStarted(ex);
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
        _url = _connection->getUrl() + "/poll";
        _url += TransportHelper::getReceiveQueryString(_connection, _connection->onSending(), getTransportType());
        connect(_httpClient, SIGNAL(getRequestCompleted(QString,SignalException*)), this, SLOT(onPollHttpResponse(QString,SignalException*)));
        _httpClient->get(_url);
        _keepAliveTimer.start();
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
    _keepAliveTimer.stop();
    _httpClient->abort(false);
}
