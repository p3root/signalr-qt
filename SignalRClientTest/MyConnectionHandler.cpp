#include "MyConnectionHandler.h"
#include <iostream>

MyConnectionHandler::MyConnectionHandler(void)
{
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
}

void MyConnectionHandler::onStateChanged(Connection::State old_state, Connection::State new_state)
{
    QLOG_DEBUG()  << "state changed: " << old_state << " -> " << new_state;
}
