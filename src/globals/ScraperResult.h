#pragma once

#include <QString>
#include <QDate>
#include <QDebug>

struct ScraperSearchResult
{
    QString id;
    QString id2;
    QString name;
    QDate released;
};

QDebug operator<<(QDebug lhs, const ScraperSearchResult& rhs);
