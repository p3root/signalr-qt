#ifndef HELPER_H
#define HELPER_H
#include <QtCore>

class Helper
{
public:
    Helper();

    static QString encode(QString val);
    static QString getEncodedQueryString(QUrl);

    static void wait(int seconds);
};

#endif // HELPER_H
