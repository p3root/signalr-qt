#include "EmptyProtectionData.h"
#include <QObject>

EmptyProtectionData::EmptyProtectionData()
{
}

QString EmptyProtectionData::protect(const QString &data, const QString &purpose)
{
    Q_UNUSED(purpose);
    return data;
}

QString EmptyProtectionData::unprotect(const QString &protectedData, const QString &purpose)
{
    Q_UNUSED(purpose);
    return protectedData;
}
