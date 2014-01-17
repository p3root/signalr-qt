#ifndef PRINCIPAL_H
#define PRINCIPAL_H

#include <QString>

class Principal
{
public:
    Principal();
    Principal(QString name, QString authenticationType, bool authenticated);

    const QString &getName() const { return _name; }
    const QString &getAuthenticationType() const { return _authenticationType; }
    bool getIsAuthenticated() const { return _isAuthenticated; }

private:
    QString _name;
    QString _authenticationType;
    bool _isAuthenticated;
};

#endif // PRINCIPAL_H
