#include "Client.h"

#include <QCoreApplication>


Client::Client(QCoreApplication &app)
{
    _timer.setSingleShot(true);
    _timer.setInterval(400000);
    _timer.start();
    connect(&_timer, SIGNAL(timeout()), this, SLOT(timerTick()));
    connect(&app, SIGNAL(aboutToQuit()), SLOT(stop()));
}

Client::~Client()
{
    delete _handler;
    delete _connection;
    delete _client;
}

void Client::start()
{
    QLOG_DEBUG() << "Client Thread: " << thread()->currentThreadId();
    _handler = new MyConnectionHandler();
    _connection = new HubConnection("http://patrik.pfaffenbauer.at:8888/signalr", _handler);

    _client = new HttpClient();
    _transport = new ServerSentEventsTransport(_client, _connection);

    HubProxy* proxy = _connection->createHubProxy("Chat");
    _connection->start(_transport, true);

    _handler->setHubProxy(proxy);
    _handler->setConnection(_connection);

    Helper::wait(1);

    proxy->invoke("send", "0");
}

void Client::stop()
{
    _connection->stop();
}

void Client::timerTick()
{
    qApp->exit(3);
}

