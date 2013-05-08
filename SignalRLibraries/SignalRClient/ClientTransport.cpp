#include "ClientTransport.h"


ClientTransport::ClientTransport(Connection* con) : _connection(0)
{
    _connection = con;
}

ClientTransport::~ClientTransport(void)
{
}
