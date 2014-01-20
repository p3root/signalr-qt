#ifndef MESSAGEBUS_H
#define MESSAGEBUS_H

#include "IMessageBus.h"
#include "Configuration/ConfigurationManager.h"

class MessageBus : public IMessageBus
{
    Q_OBJECT

public:
    MessageBus(const ConfigurationManager &configurationManager);

private:
    const ConfigurationManager &_configurationManager;
};

#endif // MESSAGEBUS_H
