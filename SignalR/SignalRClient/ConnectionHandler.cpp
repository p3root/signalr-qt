#include "ConnectionHandler.h"


ConnectionHandler::ConnectionHandler(void)
{
    connect(this, SIGNAL(onReceivedData(QVariant)), this, SLOT(onReceived(QVariant)), Qt::AutoConnection);
}


ConnectionHandler::~ConnectionHandler(void)
{
}


void ConnectionHandler::receivedData(QVariant data)
{
    emit onReceivedData(data);
}
