#include "HttpEventStreamParserTest.h"

#include <QtTest>

void HttpEventStreamParserTest::prepare(P3::SignalR::Client::HttpEventStreamParser &p, int httpStatusCode)
{
    QString httpData = QString("HTTP/1.1 %1 OK\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nTransfer-Encoding: chunked\r\nContent-Type: text/event-stream\r\nExpires: -1\r\nServer: Microsoft-IIS/7.5\r\nX-Content-Type-Options: nosniff\r\nX-AspNet-Version: 4.0.30319\r\nX-Powered-By: ASP.NET\r\nDate: Fri, 06 Jun 2014 06:33:53 GMT\r\n\r\n").arg(httpStatusCode);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
    p.addData(httpData.toHtmlEscaped().toLatin1());
#else
    p.addData(httpData.toAscii());
#endif
}

void HttpEventStreamParserTest::testSinglePacket()
{
    P3::SignalR::Client::HttpEventStreamParser p;
    P3::SignalR::Client::HttpEventStreamParserResult r;

    prepare(p);

    p.addData("3\r\n");

    QCOMPARE(p.next(r), false);
    QCOMPARE(r.error.isNull(), true);
    QCOMPARE(r.packet.isEmpty(), true);

    p.addData("x\n\n");

    QCOMPARE(p.next(r), true);
    QCOMPARE(r.error.isNull(), true);
    QCOMPARE(QString(r.packet), QString("x"));
}

void HttpEventStreamParserTest::testMultiPacket()
{
    P3::SignalR::Client::HttpEventStreamParser p;
    P3::SignalR::Client::HttpEventStreamParserResult r;

    prepare(p);

    p.addData("3\r\n");

    QCOMPARE(p.next(r), false);
    QCOMPARE(r.error.isNull(), true);
    QCOMPARE(r.packet.isEmpty(), true);

    p.addData("xyz");

    QCOMPARE(p.next(r), false);
    QCOMPARE(r.error.isNull(), true);
    QCOMPARE(r.packet.isEmpty(), true);

    p.addData("5\r\nabc\n\n");

    QCOMPARE(p.next(r), true);
    QCOMPARE(r.error.isNull(), true);
    QCOMPARE(QString(r.packet), QString("xyzabc"));
}

void HttpEventStreamParserTest::testMultiPacketInMultipleChunks()
{
    P3::SignalR::Client::HttpEventStreamParser p;
    P3::SignalR::Client::HttpEventStreamParserResult r;

    prepare(p);

    foreach (QChar  c, QByteArray("3\r\nxyz5\r\nabc\n"))
    {
        QByteArray arr;
        arr.append(c);
        p.addData(arr);
        QCOMPARE(p.next(r), false);
        QCOMPARE(r.error.isNull(), true);
        QCOMPARE(r.packet.isEmpty(), true);
    }

    p.addData("\n");

    QCOMPARE(p.next(r), true);
    QCOMPARE(r.error.isNull(), true);
    QCOMPARE(QString(r.packet), QString("xyzabc"));
}

void HttpEventStreamParserTest::testHttpError()
{
    P3::SignalR::Client::HttpEventStreamParser p;
    P3::SignalR::Client::HttpEventStreamParserResult r;

    prepare(p, 404);

    QCOMPARE(p.next(r), true);
    QCOMPARE(r.error.isNull(), false);
}

void HttpEventStreamParserTest::testInvalidPacketLength()
{
    P3::SignalR::Client::HttpEventStreamParser p;
    P3::SignalR::Client::HttpEventStreamParserResult r;

    prepare(p);

    p.addData("-3\r\nabc");

    QCOMPARE(p.next(r), false);
    QCOMPARE(r.error.isNull(), true);
    QCOMPARE(r.packet.isEmpty(), true);
}
