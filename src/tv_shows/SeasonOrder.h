#pragma once

#include <QHash>
#include <ostream>

enum class SeasonOrder : int
{
    Aired = 1,
    Dvd = 2
};

inline uint qHash(SeasonOrder order, uint seed)
{
    return qHash(static_cast<int>(order), seed);
}

std::ostream& operator<<(std::ostream& os, SeasonOrder order);
QDebug operator<<(QDebug debug, SeasonOrder id);
