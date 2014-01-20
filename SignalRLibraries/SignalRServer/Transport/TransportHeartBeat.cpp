#include "TransportHeartBeat.h"

TransportHeartBeat::TransportHeartBeat(ConfigurationManager &config) : _configurationManager(config)
{
    _timer.setInterval(config.getHeartBeatInterval());
    connect(&_timer, SIGNAL(timeout()), this, SLOT(beat()));
    _timer.start();
}

void TransportHeartBeat::beat()
{

}
