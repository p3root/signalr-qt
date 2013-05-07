#include "SignalException.h"

SignalException::SignalException() : _message("")
{
    _type = UnkownError;
}

SignalException::SignalException(const QString message, SignalExceptionType type) : _message(message)
{
    _type = type;
}

SignalException::~SignalException() throw()
{
}

const char *SignalException::what() const throw()
{
    return _message.toStdString().c_str();
}
