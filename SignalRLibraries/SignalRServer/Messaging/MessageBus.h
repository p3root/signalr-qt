#ifndef MESSAGEBUS_H
#define MESSAGEBUS_H

#include "IMessageBus.h"
#include "Configuration/ConfigurationManager.h"
#include "Infrastructure/StringMinifier.h"
#include <QTimer>

namespace P3 { namespace SignalR { namespace Server {

#define GC_TIMER_TICK_MS 5000

class MessageBus : public IMessageBus
{
    Q_OBJECT

public:
    MessageBus(const ConfigurationManager &configurationManager, int maxTopicsWithoutSubscriptions = 1000);
    ~MessageBus();

    void publish(const Message &message);
    void subscribe(void *subscrivber, QString cursor, int maxMessages, void *state);

private Q_SLOTS:
    void garbageCollectTopics();

private:
    const ConfigurationManager &_configurationManager;
    IStringMinifier *_stringMinifier;
    int _maxTopicsWithoutSubscriptions;
    QTimer _gcTimer;
    int _gcRunning;
    int _topicTtl;
};

}}}

#endif // MESSAGEBUS_H
