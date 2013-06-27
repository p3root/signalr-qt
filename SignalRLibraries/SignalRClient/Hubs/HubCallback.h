#ifndef HUBCALLBACK_H
#define HUBCALLBACK_H

#include <QObject>
#include <QVariant>

class HubCallback : public QObject
{
    Q_OBJECT
public:
    HubCallback(void* state);

    void* getState();
    void raiseMessageReceived(QVariant);

Q_SIGNALS:
    void messageReceived(HubCallback*, QVariant);

private:
    void* _state;
};

#endif // HUBCALLBACK_H
