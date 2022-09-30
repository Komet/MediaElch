#include "utils/Random.h"

#include "utils/Meta.h"

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#    include <QTime>
#else
#    include <QRandomGenerator>
#endif

namespace mediaelch {

unsigned randomUnsignedInt()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    int time = QTime::currentTime().msec();
    MediaElch_Assert(time >= 0);
    qsrand(static_cast<unsigned>(time));
    return static_cast<unsigned>(qrand());
#else
    return QRandomGenerator::global()->generate();
#endif
}

} // namespace mediaelch
