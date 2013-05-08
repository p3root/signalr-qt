#ifndef TRANSPORTHELPER_H
#define TRANSPORTHELPER_H

#include <QString>
#include "Transports/HttpClient.h"
#include "ClientTransport.h"
#include "Connection.h"
#include "SignalException.h"
#include <QtExtJson.h>


class TransportHelper : public QObject
{
    Q_OBJECT
public:
    TransportHelper(void);
    ~TransportHelper(void);


    static QString getReceiveQueryString(Connection* connection, QString data, QString transport);
    static void processMessages(Connection* connection, QString raw, bool* timedOut, bool* disconnected);


    static const NegotiateResponse* parseNegotiateHttpResponse(const QString& httpResponse);
};

#endif
