#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include "Infrastructure/HostContext.h"

namespace P3 { namespace SignalR { namespace Server {

class Connection : public QObject
{
public:
    Connection();
};
}}}
#endif // CONNECTION_H
