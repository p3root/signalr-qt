#ifndef IPROTECTDATA_H
#define IPROTECTDATA_H

#include <QString>

class IProtectedData
{
public:
    virtual QString protect(const QString &data, const QString &purpose) = 0;
    virtual QString unprotect(const QString &protectedData, const QString &purpose) = 0;
};

#endif // IPROTECTDATA_H
