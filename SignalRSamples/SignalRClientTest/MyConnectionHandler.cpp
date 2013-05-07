#include "MyConnectionHandler.h"
#include <iostream>
#include <Hubs/HubConnection.h>

MyConnectionHandler::MyConnectionHandler(void)
{
    _count = 0;
}


MyConnectionHandler::~MyConnectionHandler(void)
{
}

void MyConnectionHandler::onError(SignalException error)
{
    QLOG_DEBUG() << error.what();
}

void MyConnectionHandler::onReceived(QVariant data)
{
    QLOG_DEBUG() << data;
    _count++;
    if(_proxy)
    {
        HubProxy prox = _con->getByName("Chat");
        //prox.invoke("send", QString::number(_count));
    }
}

void MyConnectionHandler::onStateChanged(Connection::State old_state, Connection::State new_state)
{
    QLOG_DEBUG()  << "state changed: " << old_state << " -> " << new_state;
}
