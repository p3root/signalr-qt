#ifndef TRANSPORTHEARTBEAT_H
#define TRANSPORTHEARTBEAT_H

#include <QObject>
#include <QTimer>

#include "Configuration/ConfigurationManager.h"

class TransportHeartBeat : public QObject
{
    Q_OBJECT

public:
    TransportHeartBeat(ConfigurationManager &config);

private Q_SLOTS:
    void beat();

private:
    const ConfigurationManager &_configurationManager;
    QTimer _timer;
};

#endif // TRANSPORTHEARTBEAT_H
