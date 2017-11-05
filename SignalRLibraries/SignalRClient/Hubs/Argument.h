#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <QObject>


class Argument: public QGenericArgument
{
public:
    inline Argument(const char *aName, const void* aData)
        : QGenericArgument(aName, static_cast<const void *>(aData))
        {

        }
};
#endif // ARGUMENT_H
