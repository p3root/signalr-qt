#include "StringMinifier.h"
#include <QDebug>

#define BUFFER_MAX_SIZE 6

namespace P3 { namespace SignalR { namespace Server {

StringMinifier::StringMinifier() :  _locker(QMutex::Recursive)
{
    _lastMinifiedKey = -1;

}

StringMinifier::~StringMinifier()
{

}

QString StringMinifier::minify(QString &value)
{
    if(!_stringMinifier.contains(value))
    {
        _stringMinifier.insert(value, createMinifiedString(value));
    }

    return _stringMinifier[value];
}

QString StringMinifier::unminify(QString &value)
{
    if(_stringMaxifier.contains(value))
    {
        return _stringMaxifier[value];
    }
    return "";
}

void StringMinifier::removeUnminified(QString &value)
{
    if(!_stringMinifier.contains(value))
    {
        QString minifiedString = _stringMinifier[value];
        _stringMinifier.remove(value);
        _stringMaxifier.remove(minifiedString);
    }
}

QString StringMinifier::createMinifiedString(QString &fullString)
{
    QMutexLocker l(&_locker);
    Q_UNUSED(l)

    QString minString = getStringFromInt(++_lastMinifiedKey);
    _stringMaxifier.insert(minString, fullString);
    return minString;

}

QString StringMinifier::getStringFromInt(quint32 num)
{
    char buffer[BUFFER_MAX_SIZE];
    bzero(&buffer, BUFFER_MAX_SIZE);

    int index = BUFFER_MAX_SIZE;

    do
    {
        buffer[--index] = getCharFromSixBitInt(num & 0x3f);
        num >>=6;

    } while(num != 0);
    QString retVal = "";

    for(int x = BUFFER_MAX_SIZE - index; x > 0; x--)
    {
        retVal.append(QString::fromAscii(buffer+index, 1));
        index--;
    }

    return retVal;
}

char StringMinifier::getCharFromSixBitInt(quint32 num)
{
    if(num < 26)
    {
        return num + 'A';
    }
    if(num < 52)
    {
        return num - 26 + 'a';
    }
    if(num < 62)
    {
        return num - 52 + '0';
    }
    if(num == 62)
    {
        return '_';
    }
    if(num == 63)
    {
        return ':';
    }

    qWarning() << "IndexOutOfRange in StringMinifier.cpp";
    return 0;
}

}}}
