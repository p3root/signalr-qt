#include "ConfigurationManager.h"

const int missedTimeoutsBeforeClientReconnect = 2;

ConfigurationManager::ConfigurationManager()
{
    _keepAlive = 10;
    _disconnectTimeout = 30;
    _transportConnectTimeout = 5;
    _longPollDelay = 0;
    _connectionTimeout = 110;
}

int ConfigurationManager::getKeepAliveTimeout()
{
    return _keepAlive * missedTimeoutsBeforeClientReconnect;
}
