#include "Principal.h"

Principal::Principal()
{
    _name = "";
    _authenticationType = "";
    _isAuthenticated = false;
}

Principal::Principal(QString name, QString authenticationType, bool authenticated)
{
    _name = name;
    _authenticationType = authenticationType;
    _isAuthenticated = authenticated;
}
