#include "media/FileFilter.h"

namespace mediaelch {

bool FileFilter::isFileExcluded(const QString& filename) const
{
    for (const QRegularExpression& rx : fileExcludes) {
        if (rx.isValid() && rx.match(filename).hasMatch()) {
            return true;
        }
    }
    return false;
}

bool FileFilter::isFolderExcluded(const QString& folder) const
{
    for (const QRegularExpression& rx : folderExcludes) {
        if (rx.isValid() && rx.match(folder).hasMatch()) {
            return true;
        }
    }
    return false;
}


} // namespace mediaelch
