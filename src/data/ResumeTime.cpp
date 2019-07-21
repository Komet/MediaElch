#include "data/ResumeTime.h"

#include <QString>

QDebug operator<<(QDebug debug, const mediaelch::ResumeTime& time)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "ResumeTime(position=" << time.position << ", total=" << time.total << ')';
    return debug;
}
