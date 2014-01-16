#ifndef EMPTYPROTECTIONDATA_H
#define EMPTYPROTECTIONDATA_H

#include "IProtectedData.h"
#include <QString>

class EmptyProtectionData : public IProtectedData
{
public:
    EmptyProtectionData();

    virtual QString protect(const QString &data, const QString &purpose);
    virtual QString unprotect(const QString &protectedData, const QString &purpose);
};

#endif // EMPTYPROTECTIONDATA_H
