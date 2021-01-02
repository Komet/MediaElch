#include "music/TheAudioDbId.h"

#include <QRegularExpression>
#include <QString>
#include <utility>

TheAudioDbId::TheAudioDbId(QString TheAudioDbId) :
    m_TheAudioDbId(std::move(TheAudioDbId)), m_isValid{isValidFormat(m_TheAudioDbId)}
{
}

const TheAudioDbId TheAudioDbId::NoId = TheAudioDbId();

bool TheAudioDbId::operator==(const TheAudioDbId& other) const
{
    return m_TheAudioDbId == other.m_TheAudioDbId;
}

bool TheAudioDbId::operator!=(const TheAudioDbId& other) const
{
    return !(*this == other);
}

QString TheAudioDbId::toString() const
{
    return m_TheAudioDbId;
}

bool TheAudioDbId::isValid() const
{
    return m_isValid;
}

bool TheAudioDbId::isValidFormat(const QString& TheAudioDbId)
{
    return !TheAudioDbId.isEmpty();
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
