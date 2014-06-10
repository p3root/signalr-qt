#ifndef HTTPEVENTSTREAMPARSERTEST_H
#define HTTPEVENTSTREAMPARSERTEST_H

#include <Transports/HttpEventStreamParser.h>


class HttpEventStreamParserTest : public QObject
{
    Q_OBJECT

private:
    void prepare(P3::SignalR::Client::HttpEventStreamParser &p, int httpStatusCode=200);

private Q_SLOTS:
    void testSinglePacket();
    void testMultiPacket();
    void testMultiPacketInMultipleChunks();
    void testHttpError();
    void testInvalidPacketLength();
};


#endif // HTTPEVENTSTREAMPARSERTEST_H
