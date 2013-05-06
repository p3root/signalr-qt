#ifndef IHTTPRESPONSE_H
#define IHTTPRESPONSE_H

#include <QString>
#include <SignalException.h>

class HttpResponse
{
public:
    HttpResponse(void);
    ~HttpResponse(void);

    typedef void (*READ_CALLBACK)(QString data, SignalException* error, void* state);

    //virtual QString getResponseBody() = 0;
    //virtual int getStatusCode() = 0;
    
    virtual void readLine(READ_CALLBACK readCallback, void* state = 0) = 0;
};

#endif
