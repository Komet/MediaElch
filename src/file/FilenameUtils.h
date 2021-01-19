#pragma once

#include <QString>

namespace mediaelch {
namespace file {

/// \brief   Extracts the basename + path of the media file and removes meta data.
/// \details Media files can refer to the same file and can have a "-part1" suffix.
///          This function removes such meta data so that the basename can be compared.
///          This function does _NOT_ remove the file path, hence the "stacked".
QString stackedBaseName(const QString& fileName);

} // namespace file
} // namespace mediaelch
