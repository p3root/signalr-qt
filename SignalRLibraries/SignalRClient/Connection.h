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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QString>
#include <QDateTime>

#include "SignalR.h"
#include "SignalR_global.h"

#include "SignalException.h"
#include "ClientTransport.h"
#include "HeartbeatMonitor.h"
#include "KeepAliveData.h"

#ifndef QT_NO_NETWORKPROXY
    #include <QNetworkProxy>
#endif
#ifndef QT_NO_SSL
    #include <QSslConfiguration>
#endif


namespace P3 { namespace SignalR { namespace Client {

class ConnectionPrivate;

class SIGNALR_EXPORT Connection : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Connection)
    Q_DECLARE_PRIVATE(Connection)

public:
    Connection(const QString &url);
    virtual ~Connection(void);

    ///
    /// \brief Starts the connection
    /// \param AutoReconnect if set to true, the transport will ever try to reconnect automaticaly
    ///
    virtual void start(bool autoReconnect = false);

    ///
    /// \brief Starts the connection
    /// \param The transport which the connection will use
    /// \param AutoReconnect if set to true, the transport will ever try to reconnect automaticaly
    ///
    virtual void start(ClientTransport* tranport, bool autoReconnect = false);

    ///
    /// \brief Stops the connection and sends an abort to the server
    /// \param The timeout
    /// \return
    ///
    virtual bool stop(int timeoutMs=0);

    ///
    /// \brief Sends data async over the connection
    /// \param The data to send
    ///
    virtual void send(const QString &data);

    ///
    /// \brief Aborts the current retry wait, and tries to reconnect
    ///
    void retry();
    
    ///
    /// \brief Returns the current connection state
    /// \return Connection::State
    ///
    SignalR::State getState();

    ///
    /// \brief Returns the current connection id
    /// \return QString
    ///
    const QString &getConnectionId() const;

    ///
    /// \brief Returns the current connection token
    /// \return QString
    ///
    const QString &getConnectionToken() const;

    ///
    /// \brief Returns the current group token
    /// \return QString
    ///
    const QString &getGroupsToken() const;

    ///
    /// \brief Returns the current used ClientTransport
    /// \return ClientTransport
    ///
    ClientTransport *getTransport();

    ///
    /// \brief Returns the given SignalR Url
    /// \return QString
    ///
    const QString &getUrl() const;

    ///
    /// \brief Presets the used message cound
    /// \param Count which should be used
    ///
    void presetCount(quint64 preset);

    ///
    /// \brief Returns if AutoReconnect is used
    /// \return bool
    ///
    bool getAutoReconnect() const;

    const QList<QPair<QString, QString> > &getAdditionalHttpHeaders();
    void setAdditionalHttpHeaders(QList<QPair<QString, QString> > lst);

    const QList<QPair<QString, QString> > &getAdditionalQueryString();
    void setAdditionalQueryString(QList<QPair<QString, QString> > lst);

    const KeepAliveData *getKeepAliveData();

#ifndef QT_NO_NETWORKPROXY
    void setProxySettings(const QNetworkProxy proxy);
    const QNetworkProxy &getProxySettings();
#endif

    int getReconnectWaitTime();
    void setReconnectWaitTime(int timeInMilliSeconds);

    const QString &getProtocolVersion();

#ifndef QT_NO_SSL
    bool ignoreSslErrors();
    void setIgnoreSslErrors(bool ignoreSslErrors);

    void setSslConfiguration(const QSslConfiguration &config);
    const QSslConfiguration &getSslConfiguration();
#endif

    HeartbeatMonitor &getHeartbeatMonitor();

    int getPostTimeoutMs();
    void setPostTimeOutMs(int timeoutMs);

    const QString &getTid();
    void setTid(const QString &tid);

//START: SYSTEMTERA CODE
    void setMessageRepeatReconInterval(int intervalMs=60000);
    int messageRepeatReconInterval();

    void setMessageRepeatReconAmount(int amount=10);
    int messageRepeatReconAmount();
//END: SYSTEMTERA_CODE

Q_SIGNALS:
    void stateChanged(SignalR::State old_state, SignalR::State new_state);
    void errorOccured(QSharedPointer<SignalException> error);
    void onConnectionSlow();
    void logMessage(QString, int severity);
    void messageSentCompleted(QSharedPointer<SignalException> ex);
    void exceptionReceived(QString message, QString stackTrace);
    void keepAliveReceived();
    void retryReceived();

protected:
    ConnectionPrivate * getConnectionPrivate() { return d_ptr; }

    virtual void onTransportStarted(QSharedPointer<SignalException>);
    virtual void onMessageSentCompleted(QSharedPointer<SignalException>, quint64 messageId);
    virtual void onReceived(QVariant &data);
    virtual void onStateChanged(SignalR::State oldState, SignalR::State newState) { Q_UNUSED(oldState); Q_UNUSED(newState);}
    virtual QString onSending();

private Q_SLOTS:
    void transportStarted(QSharedPointer<SignalException> ex);
    void transportMessageSent(QSharedPointer<SignalException> ex, quint64 messageId);

protected:
    ConnectionPrivate * const d_ptr;

};

}}}

#endif
