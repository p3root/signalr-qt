#include "Client.h"

#include <QCoreApplication>


Client::Client(QCoreApplication &app)
{
    _timer.setSingleShot(true);
    _timer.setInterval(30000);
   // _timer.start();
    connect(&_timer, SIGNAL(timeout()), this, SLOT(timerTick()));
    connect(&app, SIGNAL(aboutToQuit()), SLOT(stop()));
}

Client::~Client()
{
    delete _connection;
    delete _client;
}

void Client::start()
{
    QLOG_DEBUG() << "Client Thread: " << thread()->currentThreadId();
    _connection = new HubConnection("http://192.168.0.202:8080/signalr");

    _client = new HttpClient();
    _transport = new LongPollingTransport(_client, _connection);

    HubProxy* proxy = _connection->createHubProxy("Chat");

    connect(proxy, SIGNAL(hubMessageReceived(QVariant)), this, SLOT(onHubMessageReceived(QVariant)));

    connect(_connection, SIGNAL(errorOccured(SignalException)), this, SLOT(onError(SignalException)));
    connect(_connection, SIGNAL(stateChanged(Connection::State,Connection::State)), this, SLOT(onStateChanged(Connection::State,Connection::State)));

    _connection->start(_transport, true);
}

void Client::stop()
{
    _connection->stop();
}

void Client::onHubMessageReceived(QVariant v)
{
    QLOG_DEBUG() << v;
}

void Client::onError(SignalException error)
{
     QLOG_DEBUG() << error.what();
}

void Client::onStateChanged(Connection::State oldState, Connection::State newState)
{
    QLOG_DEBUG()  << "state changed: " << oldState << " -> " << newState;

    if(newState == Connection::Connected)
    {
        HubCallback* callback = new HubCallback(0);
        connect(callback, SIGNAL(messageReceived(HubCallback*,QVariant)), this, SLOT(answerReceived(HubCallback*,QVariant)));
        HubProxy* prox = _connection->getByName("Chat");
        prox->invoke("send", "test", callback);
    }
}

void Client::answerReceived(HubCallback *c, QVariant v)
{



    delete c; //VERY IMPORTANT, otherwise the callback will not be delete -> memory leak
}

void Client::timerTick()
{
    qApp->exit(3);
}

