#include "TmdbId.h"

#include <QRegularExpression>
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
    return TmdbId::isValidFormat(m_tmdbId);
}

bool TmdbId::isValidPrefixedFormat(const QString& tmdbId)
{
    static QRegularExpression rx("^tmdb\\d+$");
    QRegularExpressionMatch match = rx.match(tmdbId);
    // id is greater 0
    return match.hasMatch() && tmdbId != "tmdb0";
}

bool TmdbId::isValidFormat(const QString& tmdbId)
{
    static QRegularExpression rx("^(?:tmdb)?\\d+$");
    QRegularExpressionMatch match = rx.match(tmdbId);
    // id is greater 0
    return match.hasMatch() && tmdbId != "0" && tmdbId != "tmdb0";
}

QString TmdbId::removePrefix(const QString& tmdbId)
{
    if (!tmdbId.startsWith("tmdb")) {
        return "";
    }
    // Remove "tmdb"
    return tmdbId.mid(4);
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
