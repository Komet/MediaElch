#include "RenamerUtils.h"

#include "log/Log.h"
#include "utils/Containers.h"

#include <QSet>

namespace mediaelch {

#ifdef QT_DEBUG
void ensureDataEntriesMatchPlaceholders(const char* variant,
    const QList<QString>& dataKeys,
    const QVector<Placeholder>& placeholders)
{
    QSet<QString> placeholderSet;
    for (const auto& p : placeholders) {
        placeholderSet.insert(p.name);
    }

    const QSet<QString> dataKeySet = listToSet(dataKeys);

    for (const auto& placeholder : placeholders) {
        if (!placeholder.isValue && !placeholder.isCondition) {
            qCCritical(generic).nospace() << "There is a placeholder in '" << variant
                                          << "' that is neither value nor condition: " << placeholder.name;
            MediaElch_Assert("There is a placeholder that is neither value nor condition.");
        }
        if (placeholder.isValue && !dataKeySet.contains(placeholder.name)) {
            qCCritical(generic).nospace() << "There is a placeholder in '" << variant
                                          << "' for which there is no data provider: " << placeholder.name;
            MediaElch_Assert("There is a placeholder for which there is no data provider.");
        }
    }

    for (const auto& entry : dataKeySet) {
        if (!placeholderSet.contains(entry)) {
            qCCritical(generic).nospace()
                << "There is a data entry in '" << variant << "' for which there is no placeholder: " << entry;
            MediaElch_Assert("There is a data entry for which there is no placeholder.");
        }
    }
}

void ensureConditionEntriesMatchPlaceholders(const char* variant,
    const QList<QString>& conditionKeys,
    const QVector<Placeholder>& placeholders)
{
    QSet<QString> placeholderSet;
    for (const auto& p : placeholders) {
        placeholderSet.insert(p.name);
    }

    const QSet<QString> conditionKeySet = listToSet(conditionKeys);

    for (const auto& placeholder : placeholders) {
        if (placeholder.isCondition && !placeholder.isValue && !conditionKeySet.contains(placeholder.name)) {
            qCCritical(generic).nospace() << "There is a placeholder in '" << variant
                                          << "' for which there is no condition provider: " << placeholder.name;
            MediaElch_Assert("There is a placeholder for which there is no condition provider.");
        }
    }

    for (const auto& entry : conditionKeySet) {
        if (!placeholderSet.contains(entry)) {
            qCCritical(generic).nospace()
                << "There is a condition entry in '" << variant << "' for which there is no placeholder: " << entry;
            MediaElch_Assert("There is a condition entry for which there is no placeholder.");
        }
    }
}

#endif

} // namespace mediaelch
