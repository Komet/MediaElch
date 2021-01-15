#include "music/TheAudioDbId.h"

#include <QRegularExpression>
#include <QString>
#include <utility>

TheAudioDbId::TheAudioDbId(QString theAudioDbId) :
    m_theAudioDbId(std::move(theAudioDbId)), m_isValid{isValidFormat(m_theAudioDbId)}
{
}

const TheAudioDbId TheAudioDbId::NoId = TheAudioDbId();

bool TheAudioDbId::operator==(const TheAudioDbId& other) const
{
    return m_theAudioDbId == other.m_theAudioDbId;
}

bool TheAudioDbId::operator!=(const TheAudioDbId& other) const
{
    return !(*this == other);
}

QString TheAudioDbId::toString() const
{
    return m_theAudioDbId;
}

bool TheAudioDbId::isValid() const
{
    return m_isValid;
}

bool TheAudioDbId::isValidFormat(const QString& theAudioDbId)
{
    return !theAudioDbId.isEmpty();
}

std::ostream& operator<<(std::ostream& os, const TheAudioDbId& id)
{
    return os << id.toString().toStdString();
}

QDebug operator<<(QDebug debug, const TheAudioDbId& id)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "TheAudioDbId(" << id.toString() << ')';
    return debug;
}
