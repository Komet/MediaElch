#include "model/tv_show/TvShowUtils.h"

#include "globals/Helper.h"

namespace mediaelch {

QString guessTvShowTitleFromFiles(const FileList& files)
{
    const QStringList filenameParts = files.first().toString().split('/');
    QString filename = filenameParts.last();
    if (filename.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 1 && helper::isDvd(files.first())) {
            filename = filenameParts.at(filenameParts.count() - 3);
        } else if (filenameParts.count() > 2 && helper::isDvd(files.first(), true)) {
            filename = filenameParts.at(filenameParts.count() - 2);
        }
    } else if (filename.endsWith("index.bdmv", Qt::CaseInsensitive) && filenameParts.count() > 2) {
        filename = filenameParts.at(filenameParts.count() - 3);
    }

    QString suffix = files.last().fileSuffix();
    if (suffix.length() < filename.length()) {
        // The "if" exists just to ensure that the file has a proper file
        // ending and isn't e.g. `file-without-suffix`.
        filename = filename.remove(suffix);
    }
    filename.remove("BluRay", Qt::CaseInsensitive);
    filename.remove("DVD", Qt::CaseInsensitive);
    filename = filename.replace(".", " ").replace("_", " ");
    return filename.trimmed();
}

} // namespace mediaelch
