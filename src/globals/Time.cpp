#include "globals/Time.h"

namespace mediaelch {

QString secondsToTimeCode(quint32 duration)
{
    const auto seconds = static_cast<int>(duration % 60);
    duration /= 60;
    const auto minutes = static_cast<int>(duration % 60);
    duration /= 60;
    const auto hours = static_cast<int>(duration % 24);
    const auto days = static_cast<int>(duration / 24);

    if (hours == 0 && days == 0) {
        return QStringLiteral("%1:%2") //
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }

    if (days == 0) {
        return QStringLiteral("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }

    return QStringLiteral("%1d%2:%3:%4")
        .arg(days)
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}


} // namespace mediaelch
