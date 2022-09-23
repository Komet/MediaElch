#pragma once

#include <QString>

namespace mediaelch {
namespace file {

/// \brief   Extracts the basename + path of the media file and removes meta data.
/// \details Media files can refer to the same file and can have a "-part1" suffix.
///          This function removes such meta data so that the basename can be compared.
///          This function does _NOT_ remove the file path, hence the "stacked".
QString stackedBaseName(const QString& fileName);

/// \brief   Removes the file extension from the filename.
/// \details Simply removes all text after the last dot. Does not require QFileInfo().
///          This is a naive implementation and should only be used for e.g. sorting.
QString withoutExtension(const QString& fileName);

/// \brief Sorts the given filenames locale-aware without file extension.
/// \details Sort without requiring QFileInfo. Uses withoutExtension() for sorting.
void sortFilenameList(QStringList& fileNames);

} // namespace file
} // namespace mediaelch
