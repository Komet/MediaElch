#include "TmdbId.h"

#include <QRegExp>
#include <QString>
#include <utility>

TmdbId::TmdbId(QString tmdbId) : m_tmdbId(std::move(tmdbId))
{
}

TmdbId::TmdbId(int tmdbId) : TmdbId(QString::number(tmdbId))
{
}

const TmdbId TmdbId::NoId = TmdbId();

bool TmdbId::operator==(const TmdbId& other) const
{
    return m_tmdbId == other.m_tmdbId;
}

bool TmdbId::operator!=(const TmdbId& other) const
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

std::ostream& operator<<(std::ostream& os, const TmdbId& id)
{
    return os << id.toString().toStdString();
}

QDebug operator<<(QDebug debug, const TmdbId& id)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "TmdbId(" << id.toString() << ')';
    return debug;
}
