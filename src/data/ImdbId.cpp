#include "ImdbId.h"

#include <QRegExp>
#include <QString>

ImdbId::ImdbId(QString imdbId) : m_imdbId(imdbId), m_isValid{isValidFormat(m_imdbId)}
{
}

const ImdbId ImdbId::NoId = ImdbId();

bool ImdbId::operator==(const ImdbId &other)
{
    // Only valid IMDb id's are comparable
    return other.isValid() && m_imdbId == other.m_imdbId;
}

bool ImdbId::operator!=(const ImdbId &other)
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

bool ImdbId::isValidFormat(const QString &imdbId)
{
    QRegExp regex("tt\\d{7}");
    return !imdbId.isEmpty() && regex.exactMatch(imdbId);
}
