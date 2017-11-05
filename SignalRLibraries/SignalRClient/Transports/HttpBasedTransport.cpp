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

#include "HttpBasedTransport.h"
#include "Helper/Helper.h"
#include "Connection_p.h"
#include "Helper/TransportHelper.h"

namespace P3 { namespace SignalR { namespace Client {

HttpBasedTransport::HttpBasedTransport() :
    ClientTransport(),
    _sending(false)
{
    _httpClient = 0;
}


HttpBasedTransport::~HttpBasedTransport(void)
{
    _httpClient->deleteLater();
    _httpClient = 0;
}

void HttpBasedTransport::negotiateCompleted(QString data, QSharedPointer<SignalException> ex)
{
    disconnect(_httpClient, SIGNAL(getRequestCompleted(QString,QSharedPointer<SignalException>)), this, SLOT(negotiateCompleted(QString,QSharedPointer<SignalException>)));

    if(ex.isNull())
    {
        const NegotiateResponse* res = TransportHelper::parseNegotiateHttpResponse(data);

        if(res)
        {
            onNegotiatenCompleted(*res);

            _connection->negotiateCompleted(res);
            delete res;
        }
    }
    else
    {
        _connection->updateLastRetryTime();
        if(_connection->getAutoReconnect())
        {
            _connection->emitLogMessage(QString("Negotiation failed, will try it again after %1ms").arg(_connection->getReconnectWaitTime()), SignalR::Error);
            QSharedPointer<SignalException> error = QSharedPointer<SignalException>(new SignalException("Negotiation failed", ex->getType()));
            _connection->onError(error);

            _nretryTimerTimeout.stop();
            connect(&_nretryTimerTimeout, SIGNAL(timeout()), SLOT(retryNegotiation()));
            _nretryTimerTimeout.setInterval(_connection->getReconnectWaitTime());
            _nretryTimerTimeout.start();
        }
        else
        {
            QSharedPointer<SignalException> error = QSharedPointer<SignalException>(new SignalException("Negotiation failed", SignalException::InvalidNegotiationValues));
            _connection->onError(error);
            _connection->stop();
        }
    }

}

void HttpBasedTransport::retryNegotiation()
{
    _nretryTimerTimeout.stop();
    disconnect(&_nretryTimerTimeout, SIGNAL(timeout()), this, SLOT(retryNegotiation()));
    negotiate();
}

void HttpBasedTransport::retryPost()
{
    tryDequeueNextWorkItem();
}

void HttpBasedTransport::cancelPost()
{
    _connection->emitLogMessage("Cancelling Post request", SignalR::Warning);
    _httpClient->abortPost();
}

void HttpBasedTransport::onNegotiatenCompleted(const NegotiateResponse &)
{

}

void HttpBasedTransport::negotiate()
{
    QString url = _connection->getUrl() + "/negotiate?clientProtocol=1.5&connectionData=" + _connection->onSending();
    //url += TransportHelper::getReceiveQueryString(_connection, _connection->onSending(), getTransportType());

    disconnect(this, SLOT(negotiateCompleted(QString,QSharedPointer<SignalException>)));
    connect(_httpClient, SIGNAL(getRequestCompleted(QString,QSharedPointer<SignalException>)), this, SLOT(negotiateCompleted(QString,QSharedPointer<SignalException>)));
    _httpClient->get(url);
}

void HttpBasedTransport::send(QString data)
{
    QString url = _connection->getUrl() +
            "/send";

    url += TransportHelper::getReceiveQueryString(_connection, getTransportType());

    QMap<QString, QString> postData;
    postData.insert("data",data);

    if(_sending)
    {
        SendQueueItem *queueItem = new SendQueueItem();
        queueItem->connection = _connection;
        queueItem->url = url;
        queueItem->postData = postData;
        _sendQueue.append(queueItem);
    }
    else
    {
        _sending = true;
        connect(_httpClient, SIGNAL(postRequestCompleted(QString,QSharedPointer<SignalException>)), this, SLOT(onSendHttpResponse(QString,QSharedPointer<SignalException>)));
        _httpClient->post(url, postData);
        _postTimer.start();
    }
}

void HttpBasedTransport::tryDequeueNextWorkItem()
{
    // If the queue is empty then we are free to send
    _sending = _sendQueue.size() > 0;

    if(_sending)
    {
        // Grab the next work item from the queue
        SendQueueItem* workItem = _sendQueue.dequeue();

        connect(_httpClient, SIGNAL(postRequestCompleted(QString,QSharedPointer<SignalException>)), this, SLOT(onSendHttpResponse(QString,QSharedPointer<SignalException>)));
        _httpClient->post(workItem->url, workItem->postData);

        delete workItem;
    }
}

void HttpBasedTransport::setConnectionPrivate(ConnectionPrivate *connection)
{
    ClientTransport::setConnectionPrivate(connection);
    if(_httpClient) {
        _httpClient->deleteLater();
        _httpClient = 0;
    }
    _httpClient = new HttpClient(connection);

    _postTimer.setInterval(connection->getPostTimeoutMs());
    connect(&_postTimer, SIGNAL(timeout()), this, SLOT(cancelPost()));
}

void HttpBasedTransport::retry()
{
    if(_nretryTimerTimeout.isActive())
    {
        _nretryTimerTimeout.stop();
        _nretryTimerTimeout.setInterval(_connection->getReconnectWaitTime());
        _nretryTimerTimeout.start();
    }
}

void HttpBasedTransport::onSendHttpResponse(const QString httpResponse, QSharedPointer<SignalException> error)
{    
    Q_UNUSED(httpResponse);
    Q_UNUSED(error);

    _postTimer.stop();

    disconnect(_httpClient, SIGNAL(postRequestCompleted(QString,QSharedPointer<SignalException>)), this, SLOT(onSendHttpResponse(QString,QSharedPointer<SignalException>)));

    bool timedOut = false, disconnected = false;
    quint64 messageId = 0;

    if(error.isNull())
    {
        SignalR::State curState = _connection->getState();
        _connection->changeState(curState, SignalR::Connected);
        QSharedPointer<SignalException> e = TransportHelper::processMessages(_connection, httpResponse, &timedOut, &disconnected, &messageId);
        if(!e.isNull())
            error = e;
    }

    tryDequeueNextWorkItem();

    if(messageId != 0 || !error.isNull())
        Q_EMIT onMessageSentCompleted(error, messageId);
    else
    {
        _connection->emitLogMessage("MessageId 0 received", SignalR::Warning);
        Q_EMIT onMessageSentCompleted(error, messageId);
    }

}

bool HttpBasedTransport::abort(int timeoutMs)
{
    _nretryTimerTimeout.stop();
    disconnect(_httpClient, SIGNAL(postRequestCompleted(QString,QSharedPointer<SignalException>)), this, SLOT(onSendHttpResponse(QString,QSharedPointer<SignalException>)));

    QString url = _connection->getUrl() +
            "/abort";
    url += TransportHelper::getReceiveQueryString(_connection, getTransportType());

    QSharedPointer<SignalException> ex;
    _httpClient->postSync(url,QMap<QString, QString>(), ex, timeoutMs);

    _httpClient->abort();

    if(ex.isNull())
    {
        return true;
    }
    return false;
}

void HttpBasedTransport::lostConnection(ConnectionPrivate *)
{
    _httpClient->abort(false);
}

}}}
