#include "HttpEventStreamParser.h"
#include <QStringList>

namespace P3 { namespace SignalR { namespace Client {

HttpEventStreamParser::HttpEventStreamParser()
{
    reset();
}

void HttpEventStreamParser::reset()
{
    _isFirstReponse = true;
    _packageBuffer.clear();
    _curPackage.clear();
    _curPackageLeftToRead = 0;
}

void HttpEventStreamParser::addData(const QByteArray &newData)
{
    _packageBuffer.append(newData);
}

bool HttpEventStreamParser::next(HttpEventStreamParserResult &result)
{
    result.error.clear();
    result.packet.clear();

    //read http header
    if (_isFirstReponse)
    {
        int index = _packageBuffer.indexOf("\r\n\r\n");

        if (index == -1) // http header is not completly available yet
            return false;

        //remove all http header from buffer
        QStringList statusCodeSplit =  QString(_packageBuffer.left(index+4)).split("\r\n")[0].split(" ");
        if (hasHttpErrorCode(statusCodeSplit, result))
        {
            QString data = QString::fromUtf8(_packageBuffer);
            _packageBuffer.remove(0, index+8);
            data = QString::fromUtf8(_packageBuffer);
            index = data.indexOf("\r\n");
            data = data.remove(index, data.count() - index);
            _packageBuffer.clear();
            result.packet = QByteArray().append(data);
            return true;
        }

        _packageBuffer.remove(0, index+4);
        _isFirstReponse = false;
    }

    while (true)
    {
        if (_packageBuffer.isEmpty() || !getNextPacketLength())
            return false;

        int bytesToUse = std::min(_packageBuffer.size(), _curPackageLeftToRead);
        _curPackage.append(_packageBuffer.constData(), bytesToUse);

        _packageBuffer.remove(0, bytesToUse);
        _curPackageLeftToRead -= bytesToUse;

        // \n\n signalises the end of a packet which may be sent in multiple chunks (each with its own size)
        if (_curPackageLeftToRead <= 0 && (_curPackage.endsWith("\r\n\r\n") || _curPackage.endsWith("\n\n")))
        {
            _curPackage.remove(_curPackage.length()-2, 2);
            result.packet = _curPackage;
            _curPackage.clear();
            return true;
        }
    }
}

bool HttpEventStreamParser::hasHttpErrorCode(const QStringList& statusCodeSplit, HttpEventStreamParserResult &result) const
{
    int code = statusCodeSplit[1].toInt();

    QString statusName = statusCodeSplit[2];
    if(statusCodeSplit.size() > 3) {
        statusName = "";
        for(int i = 2; i < statusCodeSplit.size(); i++) {
            statusName += statusCodeSplit[i];
            statusName += " ";
        }
    }

    //TODO: be more precisely about the status codes
    //http://www.w3.org/TR/2012/CR-eventsource-20121211/ (5 Processing model lists the status codes for SSE)
    if(code >= 200 && code <= 299)
    {
        //status ok, do nothing
    }
    else if(code >= 300 && code <= 399)
    {
        //status redirect, i think sse should not send a redirect
    }
    else if(code >= 400 && code <= 499)
    {
        //client error
        result.error = QSharedPointer<SignalException>(new SignalException(statusName, SignalException::EventStreamInitFailed));
    }
    else if(code >= 500 && code <= 599)
    {
        //server error
        result.error = QSharedPointer<SignalException>(new SignalException(statusName, SignalException::EventStreamInitFailed));
    }

    return !result.error.isNull();
}

bool HttpEventStreamParser::getNextPacketLength()
{
    if (_curPackageLeftToRead != 0)
        return true;

    int index;
    do
    {
        index = _packageBuffer.indexOf("\r\n");
        if (index == 0)
            _packageBuffer.remove(0, 2);
    } while (index == 0);

    if (index == -1) // no line break available, wait for more data
        return false;

    bool ok;
    _curPackageLeftToRead = QString(_packageBuffer.left(index)).toInt(&ok, 16);

    if (_curPackageLeftToRead < 0)
    {
        _curPackageLeftToRead = 0;
        ok = false;
    }

    _packageBuffer.remove(0, index+2);
    return ok;
}

}}}
