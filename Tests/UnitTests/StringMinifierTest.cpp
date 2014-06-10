#include <StringMinifierTest.h>

StringMinifierTest::StringMinifierTest()
{
    _minifier = new StringMinifier();
}

void StringMinifierTest::minify()
{
    QString result = "A";
    QString testValue = "__SIGNALR__SERVER__";
    QString test = _minifier->minify(testValue);

    QCOMPARE(test, result);
}

void StringMinifierTest::unmify()
{
    QString testValue = "A";
    QString result = "__SIGNALR__SERVER__";
    QString test = _minifier->unminify(testValue);

    QCOMPARE(test, result);
}
