#include "music/MusicBrainzId.h"

#include <QRegularExpression>
#include <QString>
#include <utility>

MusicBrainzId::MusicBrainzId(QString musicBrainzId) :
    m_musicBrainzId(std::move(musicBrainzId)), m_isValid{isValidFormat(m_musicBrainzId)}
{
}

const MusicBrainzId MusicBrainzId::NoId = MusicBrainzId();

bool MusicBrainzId::operator==(const MusicBrainzId& other) const
{
    return m_musicBrainzId == other.m_musicBrainzId;
}

bool MusicBrainzId::operator!=(const MusicBrainzId& other) const
{
    return !(*this == other);
}

QString MusicBrainzId::toString() const
{
    return m_musicBrainzId;
}

bool MusicBrainzId::isValid() const
{
    return m_isValid;
}

bool MusicBrainzId::isValidFormat(const QString& musicBrainzId)
{
    return !musicBrainzId.isEmpty();
}

std::ostream& operator<<(std::ostream& os, const MusicBrainzId& id)
{
    return os << id.toString().toStdString();
}

QDebug operator<<(QDebug debug, const MusicBrainzId& id)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "MusicBrainzId(" << id.toString() << ')';
    return debug;
}
