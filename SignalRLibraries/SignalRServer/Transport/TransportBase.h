#ifndef ITRANSPORT_H
#define ITRANSPORT_H

#include "../Infrastructure/HostContext.h"

class TransportBase : public QObject
{
    Q_OBJECT

public:
    TransportBase(HostContext &context);

    void setConnectionId(QString connectionId) { _connectionId = connectionId; }
    const QString &getConnectionId() { return _connectionId; }


private:
    HostContext &_context;
    QString _connectionId;
};

#endif // ITRANSPORT_H
