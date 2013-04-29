#ifndef FAKEHTTPCLIENT_H
#define FAKEHTTPCLIENT_H

#include "HttpClient.h"

class FakeHttpClient : public HttpClient
{
public:
    FakeHttpClient(void);
    ~FakeHttpClient(void);

    void get(QString url, HTTP_REQUEST_CALLBACK httpRequestCallback,  void *state)=0;
    void post(QString url, QMap<QString, QString> arguments, HTTP_REQUEST_CALLBACK httpRequestCallback, void* state = 0);
};

#endif
