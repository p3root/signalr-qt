#ifndef NEGOTIATERESPONSE_H
#define NEGOTIATERESPONSE_H

#include <QString>

struct NegotiateResponse
{
    QString connectionId;
    QString connectionToken;
    QString protocolVersion;
    double disconnectTimeout;
};

#endif
