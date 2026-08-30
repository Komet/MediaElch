#pragma once

#include "utils/Meta.h"

#include <QString>

namespace mediaelch {
namespace kodi {

/// \brief Port of Kodi's `CUtil::MakeLegalFileName(name, LegalPath::WIN32_COMPAT)` (xbmc/Util.cpp).
///
/// Kodi derives the movie set information folder's name from the set name this way and always uses
/// the WIN32_COMPAT variant, on every platform; a library written on Linux must use the identical
/// mapping or a Kodi on Windows will not find it.  helper::sanitizeFileName() is not a substitute:
/// it maps ':' to a space and drops '?' and '*'.
ELCH_NODISCARD QString makeLegalFileName(QString name);

} // namespace kodi
} // namespace mediaelch
