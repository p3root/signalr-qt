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
        EventStreamSocketLost,
        CouldNotEstablishbConnection,
        ConnectionRefusedError,
        ServerClosedConnection,
        UnkownError
    };

    SignalException(QString message, SignalExceptionType type);
    virtual ~SignalException() throw();
    const char* what() const throw() ;

    SignalExceptionType getType()  { return _type; }


private:
    QString _message;
    SignalExceptionType _type;
};

#endif // SIGNALEXCEPTION_H
