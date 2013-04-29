#include "Helper.h"

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

    for(int i = 0; i < url.queryItems().count(); i++)
    {
        retVal += QString("%1=%2&").arg(url.queryItems()[i].first, encode(url.queryItems()[i].second));
    }

    retVal.remove(retVal.length()-1, 1);

    return retVal;
}
