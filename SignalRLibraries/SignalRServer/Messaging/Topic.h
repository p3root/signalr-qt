#ifndef TOPIC_H
#define TOPIC_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QReadWriteLock>

#include "Messaging/Subscription.h"
#include "Messaging/MessageStore.hpp"
#include "Messaging/Message.h"

namespace P3 { namespace SignalR { namespace Server {

#define NO_SUBSCRIPTIONS_STATE 0
#define HAS_SUBSCRIPTIONS_STATE 1
#define SUBSCRIPTION_DYING 2
#define SUBSCRIPTION_DEAD 3

class Topic : public QObject
{
    Q_OBJECT

public:
    Topic(int storeSize, int lifespan);
    ~Topic();

    const QDateTime &getLastUsed() { return _lastUsed; }
    void markUsed() { _lastUsed = QDateTime::currentDateTimeUtc(); }

    void addSubscription(Subscription &subscription);
    void removeSubscription(Subscription &subscription);

    bool isExpired();

    MessageStore<Message> &getMessageStore() { return *_store; }

private:
    int _lifespan;
    QDateTime _lastUsed;
    QList<Subscription*> _subscriptions;
    QReadWriteLock _readWriteLock;
    int _state;

    MessageStore<Message> *_store;


};

}}}

#endif // TOPIC_H
