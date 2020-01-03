#include "ImdbId.h"

#include <QRegExp>
#include <QString>
#include <utility>

ImdbId::ImdbId(QString imdbId) : m_imdbId(std::move(imdbId)), m_isValid{isValidFormat(m_imdbId)}
{
}

const ImdbId ImdbId::NoId = ImdbId();

bool ImdbId::operator==(const ImdbId& other) const
{
    return m_imdbId == other.m_imdbId;
}

bool ImdbId::operator!=(const ImdbId& other) const
{
    return !(*this == other);
}

QString ImdbId::toString() const
{
    return m_imdbId;
}

bool ImdbId::isValid() const
{
    return m_isValid;
}

bool ImdbId::isValidFormat(const QString& imdbId)
{
    QRegExp regex("tt\\d{7,8}");
    return !imdbId.isEmpty() && regex.exactMatch(imdbId);
}

std::ostream& operator<<(std::ostream& os, const ImdbId& id)
{
    return os << id.toString().toStdString();
}

QDebug operator<<(QDebug debug, const ImdbId& id)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "ImdbId(" << id.toString() << ')';
    return debug;
}
