/*
 *  Copyright (c) 2013-2014, p3root - Patrik Pfaffenbauer (patrik.pfaffenbauer@p3.co.at)
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *
 *    Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright notice, this
 *    list of conditions and the following disclaimer in the documentation and/or
 *    other materials provided with the distribution.
 *
 *    Neither the name of the {organization} nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Helper.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif
#ifdef Q_OS_WIN32
#include <windows.h>
#endif

#include "Connection_p.h"

namespace P3 { namespace SignalR { namespace Client {

Helper::Helper()
{
}

QString Helper::encode(const QString &val)
{
    return QString(QUrl::toPercentEncoding(val));
}


QString Helper::getEncodedQueryString(QUrl url, ConnectionPrivate *con)
{
    QString retVal = "";
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 2)
    QUrlQuery query(url);
#else
    QUrl query(url);
#endif


    for(int i = 0; i < query.queryItems().count(); i++)
    {
        retVal += QString("%1=%2&").arg(query.queryItems()[i].first, encode(query.queryItems()[i].second));
    }

    if(con)
    {
        for(int i = 0; i < con->getAdditionalQueryString().size(); i++)
        {
            QString first =  con->getAdditionalQueryString().at(i).first;
            QString second =  con->getAdditionalQueryString().at(i).second;

            retVal += QString("%1=%2&").arg(first, encode(second));
        }
    }

    retVal += "timestamp=" + QString::number(QDateTime::currentDateTimeUtc().toTime_t());
    //retVal.remove(retVal.length()-1, 1);

    return retVal;
}

}}}
