#include <QString>
#include <QtTest>

#include "SignalRServer.h"
#include "Infrastructure/IStringMinifier.h"
#include "Infrastructure/StringMinifier.h"


using namespace P3::SignalR::Server;


class StringMinifierTest : public QObject
{
    Q_OBJECT

public:
    StringMinifierTest();

private Q_SLOTS:
    void minify();
    void unmify();

private:
    IStringMinifier *_minifier;
};

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

QTEST_APPLESS_MAIN(StringMinifierTest)

#include "tst_stringminifier.moc"
