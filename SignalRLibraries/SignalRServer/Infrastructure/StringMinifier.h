#ifndef STRINGMINIFIER_H
#define STRINGMINIFIER_H

#include "IStringMinifier.h"
#include <QString>
#include <QMap>
#include <QMutex>

namespace P3 { namespace SignalR { namespace Server {

class StringMinifier : public IStringMinifier
{
public:
    StringMinifier();
    ~StringMinifier();

    QString minify(QString &value);
    QString unminify(QString &value);
    void removeUnminified(QString &value);

private:
    QString createMinifiedString(QString &fullString);

    QString getStringFromInt(quint32 num);
    char getCharFromSixBitInt(quint32 num);

private:
    QMap<QString, QString> _stringMinifier;
    QMap<QString, QString> _stringMaxifier;
    int _lastMinifiedKey;
    QMutex _locker;

};

}}}

#endif // STRINGMINIFIER_H
