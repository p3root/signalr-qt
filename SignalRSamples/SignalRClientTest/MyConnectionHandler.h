#ifndef MYCONNECTIONHANDLER_H
#define MYCONNECTIONHANDLER_H

#include <ConnectionHandler.h>
#include <Hubs/HubProxy.h>

class MyConnectionHandler : public ConnectionHandler
{
public:
    MyConnectionHandler(void);
    ~MyConnectionHandler(void);

    virtual void onStateChanged(Connection::State old_state, Connection::State new_state);
    virtual void onError(SignalException error);
    virtual void onReceived(QVariant data);

    void setHubProxy(HubProxy* proxy) { proxy = proxy; }
    void setConnection(Connection* con) { _con = (HubConnection*)con; }

private:
    HubProxy* _proxy;
    HubConnection* _con;
    quint64 _count;
};

#endif
