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

#ifndef CONNECTION_P_H
#define CONNECTION_P_H

#include <QObject>
#include <SignalException.h>
#include "Transports/HttpClient.h"
#include "Transports/NegotiateResponse.h"
#include "ClientTransport.h"
#include <QDateTime>
#include "KeepAliveData.h"
#include "SignalR.h"
#include "HeartbeatMonitor.h"

#ifndef QT_NO_NETWORKPROXY
    #include <QNetworkProxy>
#endif
#ifndef QT_NO_SSL
    #include <QSslConfiguration>
#endif

namespace P3 { namespace SignalR { namespace Client {

class Connection;

class ConnectionPrivate : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ConnectionPrivate)
    Q_DECLARE_PUBLIC(Connection)

public:
    explicit ConnectionPrivate(const QString &host, Connection *connectio, QObject *parent);
    virtual ~ConnectionPrivate();

    virtual void start(bool autoReconnect = false);
    virtual void start(ClientTransport* tranport, bool autoReconnect = false);
    virtual bool stop(int timeoutMs=0);
    virtual void send(const QString &data);

    void retry();

    SignalR::State getState();
    const QString &getConnectionId() const;
    const QString &getConnectionToken() const;
    const QString &getGroupsToken() const;
    ClientTransport* getTransport();
    const QString &getUrl() const;
    const QString &getMessageId() const;

    quint64 getNextCount();

    void presetCount(quint64 preset);

    bool getAutoReconnect() const;
    KeepAliveData* getKeepAliveData();

    void updateLastKeepAlive();
    void updateLastRetryTime();
    void connectionSlow();
    void changeState(SignalR::State oldState, SignalR::State newState);

    bool ensureReconnecting();

    void onError(QSharedPointer<SignalException> exp);

    virtual void onReceived(QVariant &data);
    void setGroupsToken(const QString &token);
    void setMessageId(const QString &messageId);
    void setConnectionState(NegotiateResponse negotiateResponse);
    virtual QString onSending();

    const QList< QPair<QString, QString> >& getAdditionalHttpHeaders() { return _additionalHeaders; }
    void setAdditionalHttpHeaders(QList<QPair<QString, QString> > lst);

    const QList< QPair<QString, QString> >& getAdditionalQueryString() { return _additionalQueryString; }
    void setAdditionalQueryString(QList<QPair<QString, QString> > lst);

    void negotiateCompleted(const NegotiateResponse *negotiateResponse);

#ifndef QT_NO_NETWORKPROXY
    void setProxySettings(const QNetworkProxy proxy) { _proxySettings = proxy; }
    const QNetworkProxy &getProxySettings() { return _proxySettings; }
#endif

    int getReconnectWaitTime() { return _reconnectWaitTime; }
    void setReconnectWaitTime(int timeInMilliSeconds) { _reconnectWaitTime = timeInMilliSeconds; }

    bool tryWebSockets() { return _tryWebSockets; }

    void emitLogMessage(QString, SignalR::LogSeverity severity);

    const QString &getWebSocketsUrl() { return _webSocketsUrl; }
    const QString &getProtocolVersion() { return _protocolVersion;}

#ifndef QT_NO_SSL
    bool ignoreSslErrors() { return _ignoreSslErrors; }
    void setIgnoreSslErrors(bool ignoreSslErrors) { _ignoreSslErrors = ignoreSslErrors; }

    void setSslConfiguration(const QSslConfiguration &config) { _sslConfiguration = config; }
    const QSslConfiguration &getSslConfiguration() { return _sslConfiguration; }
#endif

    int getPostTimeoutMs() { return _postTimeoutMs; }
    void setPostTimeOutMs(int timeoutMs) { _postTimeoutMs = timeoutMs; }

    const QString &getTid() { return _tid; }
    void setTid(const QString &tid) { _tid = tid; }

    HeartbeatMonitor &getHeartbeatMonitor();

    QString translateState(SignalR::State state);

//START: SYSTEMTERA CODE
    void setMessageRepeatReconInterval(int intervalMs=60000) { _messageRepeatReconInterval = intervalMs; }
    int messageRepeatReconInterval() { return _messageRepeatReconInterval; }

    void setMessageRepeatReconAmount(int amount=10) { _messageRepeatReconAmount = amount; }
    int messageRepeatReconAmount() { return _messageRepeatReconAmount; }
//END: SYSTEMTERA_CODE

Q_SIGNALS:
    void sendData(const QString data);
    void startRetry();

private Q_SLOTS:
    void onSendData(const QString data);
    void onRetry();

private Q_SLOTS:
    void transportStarted(QSharedPointer<SignalException> ex);
    void transportMessageSent(QSharedPointer<SignalException> ex, quint64 messageId);

private:
    QString _host;
    QString _connectionId;
    QString _connectionToken;
    QString _groupsToken;
    QString _messageId;
    ClientTransport* _transport;
    QList<QPair<QString, QString> > _additionalHeaders;
    QList<QPair<QString, QString> > _additionalQueryString;
    // ConnectionHandler* _handler;
    quint64 _count;
    HttpClient *_httpClient;
    bool _autoReconnect;
    KeepAliveData *_keepAliveData;
    int _reconnectWaitTime;
    bool _tryWebSockets;
    QString _webSocketsUrl;
    QString _protocolVersion;
    SignalR::State _state;
    int _postTimeoutMs;

    HeartbeatMonitor *_monitor;
    QMutex _stateLocker;
    QMutex *_messageIdLocker;

#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy _proxySettings;
#endif

#ifndef QT_NO_SSL
    bool _ignoreSslErrors;
    QSslConfiguration _sslConfiguration;
#endif

    QString _tid;
    Connection * const q_ptr;

    int _messageRepeatReconInterval;
    int _messageRepeatReconAmount;

    QDateTime _lastRetry;
};

}}}

#endif // CONNECTION_P_H
