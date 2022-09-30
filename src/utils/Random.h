#pragma once

namespace mediaelch {

/// \brief replacement for qsrand()/qrand()
/// \details Returns a pseudo-random value.  Not _actual_ randomness.
/// \note This function is currently only used to create random screenshots.
unsigned randomUnsignedInt();

} // namespace mediaelch
