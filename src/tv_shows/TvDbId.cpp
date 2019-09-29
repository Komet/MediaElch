#include "TvDbId.h"

#include <QRegExp>
#include <QString>
#include <utility>

TvDbId::TvDbId(QString tvdbId) : m_tvdbId(std::move(tvdbId))
{
}

TvDbId::TvDbId(int tvdbId) : TvDbId(QString::number(tvdbId))
{
}

const TvDbId TvDbId::NoId = TvDbId();

bool TvDbId::operator==(const TvDbId& other) const
{
    // Only valid TvDb id's are comparable
    return other.isValid() && m_tvdbId == other.m_tvdbId;
}

bool TvDbId::operator!=(const TvDbId& other) const
{
    return !(*this == other);
}

QString TvDbId::toString() const
{
    return m_tvdbId;
}

QString TvDbId::withPrefix() const
{
    return "id" + m_tvdbId;
}

bool TvDbId::isValid() const
{
    // There are/were many places where it is checked whether the id was an
    // IMDb id. We'll continue to test for it (for now).
    return !m_tvdbId.isEmpty();
}

std::ostream& operator<<(std::ostream& os, const TvDbId& id)
{
    return os << id.toString().toStdString();
}
