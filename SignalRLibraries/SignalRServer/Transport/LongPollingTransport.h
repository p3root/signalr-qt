#ifndef LONGPOLLINGTRANSPORT_H
#define LONGPOLLINGTRANSPORT_H

#include "TransportBase.h"
#include "../Infrastructure/HostContext.h"

class LongPollingTransport : public TransportBase
{
public:
    LongPollingTransport(HostContext &context);
};

#endif // LONGPOLLINGTRANSPORT_H
