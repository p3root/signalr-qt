#ifndef HTTPEVENTSTREAMPARSER_H
#define HTTPEVENTSTREAMPARSER_H

#include "SignalException.h"

#include <QByteArray>
#include <QSharedPointer>

namespace P3 { namespace SignalR { namespace Client {

struct HttpEventStreamParserResult
{
    QByteArray packet;
    QSharedPointer<SignalException> error;
};

class HttpEventStreamParser
{
public:
    HttpEventStreamParser();
    void reset();
    void addData(const QByteArray &newData);
    bool next(HttpEventStreamParserResult &result);

private:
    bool _isFirstReponse;
    QByteArray _packageBuffer;
    QByteArray _curPackage;
    int _curPackageLeftToRead;

private:
     bool hasHttpErrorCode(const QStringList&, HttpEventStreamParserResult &result) const;
     bool getNextPacketLength();
};

}}}

#endif // HTTPEVENTSTREAMPARSER_H
