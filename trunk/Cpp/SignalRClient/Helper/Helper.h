#ifndef HELPER_H
#define HELPER_H
#include <QtCore>

class Helper
{
public:
    Helper();

    static QString encode(QString val);
    static QString getEncodedQueryString(QUrl);
};

#endif // HELPER_H
