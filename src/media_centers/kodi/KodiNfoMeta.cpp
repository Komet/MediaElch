#include "media_centers/kodi/KodiNfoMeta.h"

#include "Version.h"

#include <QDateTime>

namespace mediaelch {
namespace kodi {

QString getKodiNfoComment()
{
    return QString("created on %1 - by MediaElch version %2")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"), //
            mediaelch::constants::AppVersionFullStr);
}

} // namespace kodi
} // namespace mediaelch
