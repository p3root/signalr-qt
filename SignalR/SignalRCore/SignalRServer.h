#ifndef SIGNALRSERVER_H
#define SIGNALRSERVER_H

class SignalRServer
{
public:
    SignalRServer();

    void start(int port);

    int getPort() { return _port;}

private:
    int _port;
};

#endif // SIGNALRSERVER_H
