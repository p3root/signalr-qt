#include "FakeHttpClient.h"


FakeHttpClient::FakeHttpClient(void)
{
}


FakeHttpClient::~FakeHttpClient(void)
{
}

void FakeHttpClient::get(QString url, HTTP_REQUEST_CALLBACK httpRequestCallback, void* state)
{
    Q_UNUSED(url);
    // Make a fake http request
    httpRequestCallback(NULL, NULL, state);
}

void FakeHttpClient::post(QString url, QMap<QString, QString> postData, HTTP_REQUEST_CALLBACK httpRequestCallback, void* state)
{
    Q_UNUSED(url);
    Q_UNUSED(postData);
    // Make a fake http request
    httpRequestCallback(NULL, NULL, state);
}
