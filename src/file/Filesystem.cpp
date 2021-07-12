#include "file/Filesystem.h"

#include <QDir>

namespace mediaelch {

QStringList ElchFilesystem::directories(const QString& path)
{
    QDir dir(path);
    return dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}

} // namespace mediaelch
