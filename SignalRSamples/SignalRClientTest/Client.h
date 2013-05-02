#ifndef CLIENT_H
#define CLIENT_H
#include <QObject>

class Client : public QObject
{
    Q_OBJECT
public:
    Client();
    void start();
};

#endif // CLIENT_H
