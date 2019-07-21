#pragma once

#include <QDebug>

namespace mediaelch {

/// Resume time represents a <resume> tag in Kodi NFO files
struct ResumeTime
{
    double position = 0.0;
    double total = 0.0;
};

} // namespace mediaelch

QDebug operator<<(QDebug debug, const mediaelch::ResumeTime& time);
