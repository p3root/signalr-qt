#ifndef TST_STRINGMINIFIER_H
#define TST_STRINGMINIFIER_H

#include <QString>
#include <QtTest>

#include "SignalRServer.h"
#include "Infrastructure/IStringMinifier.h"
#include "Infrastructure/StringMinifier.h"


using namespace P3::SignalR::Server;


class StringMinifierTest : public QObject
{
    Q_OBJECT

public:
    StringMinifierTest();

private Q_SLOTS:
    void minify();
    void unmify();

private:
    StringMinifier *_minifier;
};

#endif // TST_STRINGMINIFIER_H
