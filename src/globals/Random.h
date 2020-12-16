#pragma once

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#    include <QTime>
#else
#    include <QRandomGenerator>
#endif

namespace mediaelch {

/// \brief replacement for qsrand()/qrand()
/// \details Returns a pseudo-random value.  Not _actual_ randomness.
/// \note This function is currently only used to create random screenshots.
unsigned randomUnsignedInt();

} // namespace mediaelch
