#include <QCoreApplication>
#include "Client.h"
#include <QThread>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    Client *c = new Client();
    c->start();



    return a.exec();
}
