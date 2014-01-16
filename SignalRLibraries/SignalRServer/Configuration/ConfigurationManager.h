#ifndef CONFIGURATIONMANAGER_H
#define CONFIGURATIONMANAGER_H

class ConfigurationManager
{
public:
    ConfigurationManager();

    int getKeepAliveTimeout();
    void setKeepAliveTimeout(int keepAlive) { _keepAlive = keepAlive; }

    int getTransportConnectTimeout() { return _transportConnectTimeout; }
    void setTransportConnectTimeout(int transportConnectTimeout) { _transportConnectTimeout = transportConnectTimeout; }

    int getConnectionTimeout() { return _connectionTimeout; }
    void setConnectionTimeout(int connectionTimeout) { _connectionTimeout = connectionTimeout; }

    int getDisconnectTimeout() { return _disconnectTimeout; }
    void setDisconnectTimeout(int disconnectTimeout) { _disconnectTimeout = disconnectTimeout; }

    int getLongPollDelay() { return _longPollDelay; }
    void setLongPollDelay(int longPollDelay) { _longPollDelay = longPollDelay; }

private:
    int _keepAlive;
    int _transportConnectTimeout;
    int _connectionTimeout;
    int _disconnectTimeout;
    int _longPollDelay;
};

#endif // CONFIGURATIONMANAGER_H
