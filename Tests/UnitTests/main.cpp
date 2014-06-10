#include <QCoreApplication>
#include <QTest>

#include "StringMinifierTest.h"
#include "HttpEventStreamParserTest.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    bool isError = false;
    QStringList testCmd;

    isError |= QTest::qExec(new StringMinifierTest(), testCmd);
    isError |= QTest::qExec(new HttpEventStreamParserTest(), testCmd);

    return !isError;
}
