#ifndef MESSAGESTORE_H
#define MESSAGESTORE_H

#include <QObject>
#include <QMap>
#include <QMutex>

namespace P3 { namespace SignalR { namespace Server {


template <typename T> class MessageStore
{
public:
    MessageStore(quint32 capacity);

    quint64 add(T message);

    T getMessage(quint64 id);

private:
    QMap<quint64, T> _messages;
    quint64 _nextFreeMessageId;
    QMutex _locker;

};

template<typename T> MessageStore<T>::MessageStore(quint32 capacity) : _locker(QMutex::Recursive)
{
    //set a minimum size
    if(capacity < 32)
    {
        capacity = 32;
    }
    _nextFreeMessageId = 0;
}

template<typename T> quint64 MessageStore<T>::add(T message)
{
    QMutexLocker l(&_locker);
    quint64 newMessageId = _nextFreeMessageId + 1;

    _messages.insert(newMessageId, message);

    return newMessageId;
}

template<typename T>  T MessageStore<T>::getMessage(quint64 id)
{
    QMutexLocker l(&_locker);

    if(_messages.keys().contains(id))
    {
        T message = _messages.value(id);

        _messages.remove(id);
        return message;
    }
    return 0;
}

}}}

#endif // MESSAGESTORE_H
