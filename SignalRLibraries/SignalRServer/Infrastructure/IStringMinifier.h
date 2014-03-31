#ifndef ISTRINGMINIFIER_H
#define ISTRINGMINIFIER_H

#include <QString>

namespace P3 { namespace SignalR { namespace Server {

class IStringMinifier {

public:

    virtual ~IStringMinifier() {}
    virtual QString minify(QString &value) = 0;
    virtual QString unminify(QString &value) = 0;
    virtual void removeUnminified(QString &value) = 0;
};

}}}

#endif // ISTRINGMINIFIER_H
