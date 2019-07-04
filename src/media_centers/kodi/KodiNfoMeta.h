#pragma once

#include <QString>

namespace mediaelch {
namespace kodi {

/// Returns a string containing basic meta information about the
/// media manager used to create the NFO file (MediaElch)
QString getKodiNfoComment();

} // namespace kodi
} // namespace mediaelch
