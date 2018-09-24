#include "TmdbId.h"

#include <QRegExp>
#include <QString>

TmdbId::TmdbId(QString tmdbId) : m_tmdbId(tmdbId)
{
}

TmdbId::TmdbId(int tmdbId) : TmdbId(QString::number(tmdbId))
{
}

const TmdbId TmdbId::NoId = TmdbId();

bool TmdbId::operator==(const TmdbId &other)
{
    // Only valid TMDb id's are comparable
    return other.isValid() && m_tmdbId == other.m_tmdbId;
}

bool TmdbId::operator!=(const TmdbId &other)
{
    return !(*this == other);
}

QString TmdbId::toString() const
{
    return m_tmdbId;
}

QString TmdbId::withPrefix() const
{
    return "id" + m_tmdbId;
}

bool TmdbId::isValid() const
{
    // There are/were many places where it is checked whether the id was an
    // IMDb id. We'll continue to test for it (for now).
    return !m_tmdbId.isEmpty() && !m_tmdbId.startsWith("tt");
}
