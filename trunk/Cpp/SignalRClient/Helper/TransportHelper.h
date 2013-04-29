#ifndef TRANSPORTHELPER_H
#define TRANSPORTHELPER_H

#include <QString>
#include "Transports/HttpClient.h"
#include "ClientTransport.h"
#include "Connection.h"
#include "SignalException.h"
#include <QtExtJson.h>


class TransportHelper
{
public:
    TransportHelper(void);
    ~TransportHelper(void);

    static void getNegotiationResponse(HttpClient* httpClient, Connection* connnection, ClientTransport::NEGOTIATE_CALLBACK negotiateCallback, void* state = 0);
    static QString getReceiveQueryString(Connection* connection, QString data, QString transport);
    static void processMessages(Connection* connection, QString raw, bool* timedOut, bool* disconnected);

private:
    struct NegotiationRequestInfo
    {
        void* userState;
        ClientTransport::NEGOTIATE_CALLBACK callback;
    };

    static void onNegotiateHttpResponse(const QString& httpResponse, SignalException* error, void* state);
};

#endif
