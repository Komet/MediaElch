#include "media_center/kodi/MakeLegalFileName.h"

namespace mediaelch {
namespace kodi {

QString makeLegalFileName(QString name)
{
    const QString illegalCharacters = QStringLiteral(R"(/\?:*"<>|)");

    for (QChar& character : name) {
        if (illegalCharacters.contains(character)) {
            character = QLatin1Char('_');
        }
    }

    while (!name.isEmpty() && (name.endsWith(QLatin1Char('.')) || name.endsWith(QLatin1Char(' ')))) {
        name.chop(1);
    }

    return name;
}

} // namespace kodi
} // namespace mediaelch
