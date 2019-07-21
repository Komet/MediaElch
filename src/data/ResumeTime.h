#pragma once

#include <QDebug>

namespace mediaelch {

/// Resume time represents a <resume> tag in Kodi NFO files
struct ResumeTime
{
    double position;
    double total;
};

} // namespace mediaelch

QDebug operator<<(QDebug debug, const mediaelch::ResumeTime& id);
