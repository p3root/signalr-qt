#include "TopicLookup.h"
#include "Infrastructure/PrefixHelper.h"

namespace P3 { namespace SignalR { namespace Server {

TopicLookup::TopicLookup()
{
}

const Topic *TopicLookup::operator[](const QString &key) const
{
    Topic *topic = 0;

    if(tryGetValue(key, topic))
    {
        return topic;
    }
    return 0;
}

int TopicLookup::count() const
{
    return _topics.count() + _groupTopics.count();
}

bool TopicLookup::containsKey(const QString &key)
{
    if(PrefixHelper::hasGroupPrefix(key))
    {
        return _groupTopics.contains(key);
    }

    return _topics.contains(key);
}

bool TopicLookup::tryRemove(const QString &key)
{
    Topic *topic = 0;

    if(PrefixHelper::hasGroupPrefix(key))
    {
        if(tryGetValue(key, topic))
        {
            _groupTopics.remove(key);
            delete topic;
            return true;
        }
        return false;
    }

    if(tryGetValue(key, topic))
    {
        _topics.remove(key);
        delete topic;
        return true;
    }
    return false;
}

bool TopicLookup::tryGetValue(const QString &key, Topic *topic) const
{
    Q_UNUSED(topic)
    if(PrefixHelper::hasGroupPrefix(key))
    {
        if(_groupTopics.contains(key))
        {
            topic = _groupTopics.value(key);
            return true;
        }
        topic = 0;
        return false;
    }

    if(_topics.contains(key))
    {
        topic = _topics.value(key);
        return true;
    }
    topic = 0;
    return false;
}

}}}
