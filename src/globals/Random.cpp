#include "globals/Random.h"

namespace mediaelch {

unsigned randomUnsignedInt()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    qsrand(QTime::currentTime().msec());
    return static_cast<unsigned>(qrand());
#else
    return QRandomGenerator::global()->generate();
#endif
}

} // namespace mediaelch
