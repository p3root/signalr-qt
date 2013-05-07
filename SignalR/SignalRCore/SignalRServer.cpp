#include "SignalRServer.h"
#include "ConnectionManager.h"

SignalRServer::SignalRServer()
{
}

void SignalRServer::start(int port)
{
    _port = port;
    ConnectionManager* m = new ConnectionManager(*this);
    m->startListener();
}
