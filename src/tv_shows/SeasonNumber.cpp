#include "SeasonNumber.h"

#include <QDebug>
#include <QString>

SeasonNumber::SeasonNumber(int seasonNumber) noexcept :
    // Any number lower than 0 is regarded invalid => no episode number
    m_seasonNumber(seasonNumber > -2 ? seasonNumber : -2)
{
}

const SeasonNumber SeasonNumber::NoSeason = SeasonNumber(-2);
const SeasonNumber SeasonNumber::SpecialsSeason = SeasonNumber(0);

bool SeasonNumber::operator==(const SeasonNumber& other) const
{
    return m_seasonNumber == other.m_seasonNumber;
}

bool SeasonNumber::operator!=(const SeasonNumber& other) const
{
    return !(*this == other);
}

bool SeasonNumber::operator>(const SeasonNumber& other) const
{
    return m_seasonNumber > other.m_seasonNumber;
}

bool SeasonNumber::operator<(const SeasonNumber& other) const
{
    return m_seasonNumber < other.m_seasonNumber;
}

int SeasonNumber::toInt() const
{
    return m_seasonNumber;
}

QString SeasonNumber::toPaddedString() const
{
    if (m_seasonNumber == SeasonNumber::NoSeason.toInt()) {
        return QStringLiteral("xx");
    }
    return QString::number(m_seasonNumber).prepend((m_seasonNumber < 10) ? "0" : "");
}

QString SeasonNumber::toString() const
{
    return QString::number(m_seasonNumber);
}

std::ostream& operator<<(std::ostream& os, const SeasonNumber& id)
{
    return os << id.toString().toStdString();
}

QDebug operator<<(QDebug debug, const SeasonNumber& season)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "SeasonNumber(" << season.toPaddedString() << ')';
    return debug;
}
