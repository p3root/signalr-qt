#ifndef TOPICLOOKUP_H
#define TOPICLOOKUP_H

#include <QObject>
#include <QList>
#include <QPair>
#include "Messaging/Topic.h"

namespace P3 { namespace SignalR { namespace Server {

class TopicLookup : QList<QPair<QString, Topic> >
{
public:
    TopicLookup();

    const Topic *operator[](const QString &key) const;

    int count() const;

    bool containsKey(const QString &key);
    bool tryRemove(const QString &key);

  //  Topic &getOrAdd(QString key,


private:
    bool tryGetValue(const QString &key, Topic *topic) const;

private:
    QMap<QString, Topic*> _topics;
    QMap<QString, Topic*> _groupTopics;
};

}}}

#endif // TOPICLOOKUP_H
