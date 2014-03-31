#include "Topic.h"

namespace P3 { namespace SignalR { namespace Server {

Topic::Topic(int storeSize, int lifespan) : _store(0)
{
    _lifespan = lifespan;

    _store = new MessageStore<Message>(storeSize);
}

Topic::~Topic()
{
    delete _store;
}

void Topic::addSubscription(Subscription &subscription)
{
    _readWriteLock.lockForWrite();

    markUsed();

    _subscriptions.append(&subscription);

    if(_state == NO_SUBSCRIPTIONS_STATE)
        _state = HAS_SUBSCRIPTIONS_STATE;
    else
        _state = NO_SUBSCRIPTIONS_STATE;

    _readWriteLock.unlock();
}

void Topic::removeSubscription(Subscription &subscription)
{
    _readWriteLock.lockForWrite();

    markUsed();

    _subscriptions.removeOne(&subscription);

    if(_subscriptions.count() == 0)
    {
        if(_state == HAS_SUBSCRIPTIONS_STATE)
            _state = NO_SUBSCRIPTIONS_STATE;
        else
            _state = HAS_SUBSCRIPTIONS_STATE;

    }

    _readWriteLock.unlock();
}

bool Topic::isExpired()
{
    _readWriteLock.lockForRead();

    int timeDiff = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() - _lastUsed.toMSecsSinceEpoch();

    return _subscriptions.count() == 0 && timeDiff > _lifespan;
}

}}}
