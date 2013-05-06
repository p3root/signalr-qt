#ifndef NEGOTIATIONMESSAGE_H
#define NEGOTIATIONMESSAGE_H

#include <QString>
#include <QtExtJson.h>
#include <HttpRequest.h>

class NegotiationMessage
{
public:
    NegotiationMessage();

    static QString generate(HttpRequest *);
};

#endif // NEGOTIATIONMESSAGE_H
