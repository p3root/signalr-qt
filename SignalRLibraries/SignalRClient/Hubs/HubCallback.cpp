#include "HubCallback.h"

HubCallback::HubCallback(void* state) : QObject()
{
    _state = state;
}

void *HubCallback::getState()
{
    return _state;
}

void HubCallback::raiseMessageReceived(QVariant v)
{
    Q_EMIT messageReceived(this, v);
}
