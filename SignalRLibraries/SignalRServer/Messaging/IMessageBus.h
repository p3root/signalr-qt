#ifndef IMESSAGEBUS_H
#define IMESSAGEBUS_H

#include <QObject>

#include "Message.h"
#include "MessageResult.h"

namespace P3 { namespace SignalR { namespace Server {

class IMessageBus : public QObject
{
    Q_OBJECT
public:
    virtual void publish(const Message &message) = 0;
    virtual void subscribe(void *subscrivber, QString cursor, int maxMessages, void *state)=0;

Q_SIGNALS:
    void callback(const MessageResult *message, void *state);

};

}}}
#endif // IMESSAGEBUS_H
