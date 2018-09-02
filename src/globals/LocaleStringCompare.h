#ifndef LOCALESTRINGCOMPARE_H
#define LOCALESTRINGCOMPARE_H

#include <QString>

class LocaleStringCompare
{
public:
    bool operator()(const QString &s1, const QString &s2) const { return QString::localeAwareCompare(s1, s2) < 0; }
};

#endif // LOCALESTRINGCOMPARE_H
