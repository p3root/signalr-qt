#ifndef SIGNALEXCEPTION_H
#define SIGNALEXCEPTION_H

#include <QString>
#include <QMetaType>

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

    SignalException();
    SignalException(const QString message, SignalExceptionType type);
    ~SignalException() throw();
    const char* what() const throw() ;

    SignalExceptionType getType()  { return _type; }


private:
    const QString _message;
    SignalExceptionType _type;
};


Q_DECLARE_METATYPE(SignalException*)

#endif // SIGNALEXCEPTION_H
