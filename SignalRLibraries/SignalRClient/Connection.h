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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>
#include <SignalException.h>
#include "Transports/HttpClient.h"
#include "Transports/NegotiateResponse.h"
#include "ClientTransport.h"
#include <QDateTime>
#include <QObject>
#include "KeepAliveData.h"
#include <QNetworkProxy>

class ConnectionHandler;
class ClientTransport;

class Connection : public QObject
{
    Q_OBJECT
public:
    enum State
    {
        Connecting,
        Connected,
        Reconnecting,
        Disconnecting,
        Disconnected
    };

    enum LogSeverity
    {
        Debug,
        Info,
        Warning,
        Error
    };

    Connection(const QString &url);
    virtual ~Connection(void);

    void start(bool autoReconnect = false);
    void start(ClientTransport* tranport, bool autoReconnect = false);
    void start(HttpClient* client, bool autoReconnect = false);
    virtual bool stop(int timeoutMs=0);
    virtual void send(const QString &data);
    
    State getState();
    const QString &getConnectionId() const;
    const QString &getConnectionToken() const;
    const QString &getGroupsToken() const;
    ClientTransport* getTransport();
    const QString &getUrl() const;
    const QString &getMessageId() const;
    int getPort();
    quint64 getCount() const;
    bool getAutoReconnect() const;

    KeepAliveData& getKeepAliveData();
    void updateLastKeepAlive();
    void connectionSlow();

    bool changeState(State oldState, State newState);
    bool ensureReconnecting();
    void onError(SignalException exp);
    virtual void onReceived(QVariant data);

    void setGroupsToken(const QString &token) { _groupsToken = token; }
    void setMessageId(const QString &messageId) { _messageId = messageId; }
    void setConnectionState(NegotiateResponse negotiateResponse);
    virtual QString onSending();

    const QList< QPair<QString, QString> >& getAdditionalHttpHeaders() { return _additionalHeaders; }
    void setAdditionalHttpHeaders(QList<QPair<QString, QString> > lst);

    const QList< QPair<QString, QString> >& getAdditionalQueryString() { return _additionalQueryString; }
    void setAdditionalQueryString(QList<QPair<QString, QString> > lst);

    void negotiateCompleted(const NegotiateResponse *negotiateResponse, SignalException* error);

    void setProxySettings(const QNetworkProxy proxy) { _proxySettings = proxy; }
    const QNetworkProxy &getProxySettings() { return _proxySettings; }

    void setLogErrorsToQDebug(bool val)  { _logErrorsToQDebug = val; }
    bool getLogErrorsToQDebug() { return _logErrorsToQDebug; }

    int getReconnectWaitTime() { return _reconnectWaitTime; }
    void setReconnectWaitTime(int timeInSeconds) { _reconnectWaitTime = timeInSeconds; }

    bool tryWebSockets() { return _tryWebSockets; }

    void emitLogMessage(QString, LogSeverity severity);

    const QString &getWebSocketsUrl() { return _webSocketsUrl; }
    const QString &getProtocolVersion() { return _protocolVersion;}

Q_SIGNALS:
    void stateChanged(Connection::State old_state, Connection::State new_state);
    void errorOccured(SignalException error);
    void messageReceived(QVariant data);
    void onConnectionSlow();
    void logMessage(QString, int severity);

protected:
     State _state;

private Q_SLOTS:
     void transportStarted(SignalException *ex);

private:
    bool _logErrorsToQDebug;
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
    QNetworkProxy _proxySettings;
    int _reconnectWaitTime;
    bool _tryWebSockets;
    QString _webSocketsUrl;
    QString _protocolVersion;
};

#endif
