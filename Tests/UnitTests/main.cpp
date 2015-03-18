#include <QCoreApplication>
#include <QTest>

#include "HttpEventStreamParserTest.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    bool isError = false;
    QStringList testCmd;

    isError |= QTest::qExec(new HttpEventStreamParserTest(), testCmd);

    return !isError;
}
