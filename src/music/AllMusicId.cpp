#include "music/AllMusicId.h"

#include <QRegularExpression>
#include <QString>
#include <utility>

AllMusicId::AllMusicId(QString allMusicId) : m_allMusicId(std::move(allMusicId)), m_isValid{isValidFormat(m_allMusicId)}
{
}

const AllMusicId AllMusicId::NoId = AllMusicId();

bool AllMusicId::operator==(const AllMusicId& other) const
{
    return m_allMusicId == other.m_allMusicId;
}

bool AllMusicId::operator!=(const AllMusicId& other) const
{
    return !(*this == other);
}

QString AllMusicId::toString() const
{
    return m_allMusicId;
}

bool AllMusicId::isValid() const
{
    return m_isValid;
}

bool AllMusicId::isValidFormat(const QString& allMusicId)
{
    return !allMusicId.isEmpty();
}

std::ostream& operator<<(std::ostream& os, const AllMusicId& id)
{
    return os << id.toString().toStdString();
}

QDebug operator<<(QDebug debug, const AllMusicId& id)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "AllMusicId(" << id.toString() << ')';
    return debug;
}
