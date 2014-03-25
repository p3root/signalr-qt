#ifndef CONNECTIONMESSAGE_H
#define CONNECTIONMESSAGE_H

#include <QString>
#include <QList>
#include <QVariant>

namespace P3 { namespace SignalR { namespace Server {

class ConnectionMessage
{
public:
    ConnectionMessage(QList<QString> signal, QVariant value, QList<QString> excludedSignals = QList<QString>());
    ConnectionMessage(QString signal, QVariant value, QList<QString> excludedSignals = QList<QString>());

    const QString &getSignal() { return _signal; }
    const QList<QString> &getSignals() { return _signals; }
    const QList<QString> &getExcludedSignals() { return _excludedSignals; }
    const QVariant &getValue() { return _value; }

private:
    QString _signal;
    QList<QString> _signals;
    QList<QString> _excludedSignals;
    QVariant _value;
};

}}}

#endif // CONNECTIONMESSAGE_H
