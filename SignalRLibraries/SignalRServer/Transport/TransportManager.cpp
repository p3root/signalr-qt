#include "TransportManager.h"

#include "LongPollingTransport.h"

TransportManager::TransportManager()
{

}

bool TransportManager::transportExists(const QString &transportName)
{
    if(transportName == "longPolling")
        return true;

    return false;
}

TransportBase *TransportManager::createTransport(const QString &transportName, HostContext &context)
{
    if(transportName == "longPolling")
        return new LongPollingTransport(context);

    return 0;
}
