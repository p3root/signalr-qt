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

#ifndef HTTPBASEDTRANSPORT_H
#define HTTPBASEDTRANSPORT_H

#include "ClientTransport.h"
#include "SignalException.h"
#include <QQueue>
#include <QTimer>
#include <QMap>
#include "Transports/NegotiateResponse.h"
#include <QEventLoop>
#include "SignalR.h"

namespace P3 { namespace SignalR { namespace Client {

class HttpClient;

class SIGNALR_EXPORT HttpBasedTransport : public ClientTransport
{
    Q_OBJECT
protected:
    HttpClient* _httpClient;

public:
    HttpBasedTransport();
    virtual ~HttpBasedTransport(void);

    void negotiateCompleted(QString data, QSharedPointer<SignalException> ex) OVERRIDE_M;
    virtual void onNegotiatenCompleted(const NegotiateResponse& res);
    void negotiate() OVERRIDE_M;
    void send(QString data) OVERRIDE_M;
    virtual bool abort(int timeoutMs = 0) OVERRIDE_M;

    void lostConnection(ConnectionPrivate *con) OVERRIDE_M;

    virtual void retry() OVERRIDE_M;

    void tryDequeueNextWorkItem();
private:    
    
    struct SendQueueItem
    {
        ConnectionPrivate* connection;
        QString url;
        QMap<QString, QString> postData;
    };
    
    QQueue<SendQueueItem*> _sendQueue;
    bool _sending;
    QTimer _nretryTimerTimeout;
    QTimer _postTimer;


protected:
    void setConnectionPrivate(ConnectionPrivate *connection) OVERRIDE_M;

private Q_SLOTS:
    void onSendHttpResponse(const QString httpResponse, QSharedPointer<SignalException> error);
    void retryNegotiation();
    void retryPost();
    void cancelPost();
};

}}}

#endif
