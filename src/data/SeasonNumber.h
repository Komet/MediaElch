#pragma once

#include <QString>

class SeasonNumber
{
public:
    SeasonNumber() = default;
    explicit SeasonNumber(int seasonNumber);

    bool operator==(const SeasonNumber &other) const;
    bool operator!=(const SeasonNumber &other) const;
    bool operator>(const SeasonNumber &other) const;
    bool operator<(const SeasonNumber &other) const;

    int toInt() const;
    QString toPaddedString() const;
    QString toString() const;

    static const SeasonNumber NoSeason;
    static const SeasonNumber SpecialsSeason;

private:
    int m_seasonNumber = -1; // No season
};
