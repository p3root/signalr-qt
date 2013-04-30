#include "Helper.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif
#ifdef Q_OS_WIN32
#include <windows.h>
#endif

Helper::Helper()
{
}

QString Helper::encode(QString val)
{
    QString retVal = val.replace("/", "%2F");
    retVal = retVal.replace("+", "%2B");
    retVal = retVal.replace("=", "%3D");

    retVal = retVal.replace("{", "%7B");
    retVal = retVal.replace("[", "%5B");
    retVal = retVal.replace("}", "%7D");
    retVal = retVal.replace("]", "%5D");
    retVal = retVal.replace(":", "%3A");
    retVal = retVal.replace(" ", "%20");
    retVal = retVal.replace("\"", "%22");


    return retVal;
}


QString Helper::getEncodedQueryString(QUrl url)
{
    QString retVal = "";
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
    QUrlQuery query(url);
#else
    QUrl query(url);
#endif


    for(int i = 0; i < query.queryItems().count(); i++)
    {
        retVal += QString("%1=%2&").arg(query.queryItems()[i].first, encode(query.queryItems()[i].second));
    }

    retVal.remove(retVal.length()-1, 1);

    return retVal;
}

void Helper::wait(int milliseconds)
{
#ifdef Q_OS_LINUX
    sleep(milliseconds);
#endif
#ifdef Q_OS_WIN32
    Sleep(milliseconds);
#endif
}
