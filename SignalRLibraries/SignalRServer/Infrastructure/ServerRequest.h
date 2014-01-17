#ifndef SERVERREQUEST_H
#define SERVERREQUEST_H

#include <QHttpRequest.h>
#include "../Infrastructure/Principal.h"

class ServerRequest
{
public:
    ServerRequest(const QHttpRequest &req);

    const QUrl &getUrl() { return _request.url(); }
    const QString &getLocalPath() { return _path; }
    const QList<QPair<QString, QString> > &getQueryString() { return _queryItems; }
    const QHash<QString, QString> &getHeaders() { return _request.headers(); }

    const Principal &getUser() { return _user; }

private:
    const QHttpRequest &_request;
    QString _path;
    QList<QPair<QString, QString> >  _queryItems;
    Principal _user;
};

#endif // SERVERREQUEST_H
