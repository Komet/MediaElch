#pragma once

#include <QDate>
#include <QDebug>
#include <QString>

struct ScraperSearchResult
{
    QString id;
    QString id2;
    QString name;
    QDate released;
};

QDebug operator<<(QDebug lhs, const ScraperSearchResult& rhs);
