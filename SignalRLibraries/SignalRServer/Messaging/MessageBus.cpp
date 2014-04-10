#include "MessageBus.h"

namespace P3 { namespace SignalR { namespace Server {

MessageBus::MessageBus(const ConfigurationManager &configurationManager, int maxTopicsWithoutSubscriptions) : _configurationManager(configurationManager)
{
    _stringMinifier = new StringMinifier();
    _maxTopicsWithoutSubscriptions = maxTopicsWithoutSubscriptions;

    _gcTimer.setInterval(GC_TIMER_TICK_MS);
    connect(&_gcTimer, SIGNAL(timeout()), this, SLOT(garbageCollectTopics()));

    _topicTtl = configurationManager.topicTtl();
}

MessageBus::~MessageBus()
{
    delete _stringMinifier;
    _stringMinifier = 0;
}

void MessageBus::publish(const Message &message)
{
    Q_UNUSED(message);
}

void MessageBus::subscribe(void *subscrivber, QString cursor, int maxMessages, void *state)
{
    Q_UNUSED(subscrivber);
    Q_UNUSED(cursor);
    Q_UNUSED(maxMessages);
    Q_UNUSED(state);
}

void MessageBus::garbageCollectTopics()
{

}

}}}
