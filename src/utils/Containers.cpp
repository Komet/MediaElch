#include "utils/Containers.h"

namespace mediaelch {

QStringList split_string_trimmed(const QString& str, const QString& delimiter)
{
    QStringList entries = str.split(delimiter);

    for (QString& entry : entries) {
        entry = entry.trimmed();
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
    entries.removeIf([](const QString& entry) { return entry.isEmpty(); });
#else
    for (auto i = 0; i < entries.length(); ++i) {
        if (entries[i].isEmpty()) {
            entries.removeAt(i);
            --i; // entry was removed, hence i now contains the next value
        }
    }
#endif

    return entries;
}


} // namespace mediaelch
