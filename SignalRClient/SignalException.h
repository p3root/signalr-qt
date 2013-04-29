#ifndef SIGNALEXCEPTION_H
#define SIGNALEXCEPTION_H

#include <QString>

class SignalException : public std::exception
{
public:
    enum SignalExceptionType
    {
        InvalidNegotiationValues = 0,
        InvalidProtocolVersion,
        EventStreamSocketLost
    };

    SignalException(QString message, SignalExceptionType type);
    virtual ~SignalException() throw();
    const char* what() const throw() ;


private:
    QString _message;
    SignalExceptionType _type;
};

#endif // SIGNALEXCEPTION_H
