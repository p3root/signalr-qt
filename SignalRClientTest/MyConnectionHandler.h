#ifndef MYCONNECTIONHANDLER_H
#define MYCONNECTIONHANDLER_H

#include <ConnectionHandler.h>

class MyConnectionHandler : public ConnectionHandler
{
public:
    MyConnectionHandler(void);
    ~MyConnectionHandler(void);

    virtual void onStateChanged(Connection::State old_state, Connection::State new_state);
    virtual void onError(SignalException error);
    virtual void onReceived(QString data);
};

#endif
