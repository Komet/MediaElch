#pragma once

#include "RenamerPlaceholders.h"

#include <QList>
#include <QString>
#include <QVector>

#ifdef QT_DEBUG
#    define MediaElch_Ensure_Data_Matches_Placeholders(PlaceholderClass, dataMap)                                      \
        do {                                                                                                           \
            static const bool testFlag = [&dataMap]() {                                                                \
                PlaceholderClass p;                                                                                    \
                ensureDataEntriesMatchPlaceholders(#PlaceholderClass, map.keys(), p.placeholders());                   \
                return false;                                                                                          \
            }();                                                                                                       \
            Q_UNUSED(testFlag)                                                                                         \
        } while (false)
#else
#    define MediaElch_Ensure_Data_Matches_Placeholders(PlaceholderClass, dataMap)                                      \
        do {                                                                                                           \
        } while (false)
#endif


#ifdef QT_DEBUG
#    define MediaElch_Ensure_Condition_Matches_Placeholders(PlaceholderClass, conditionMap)                            \
        do {                                                                                                           \
            static const bool testFlag = [&conditionMap]() {                                                           \
                PlaceholderClass p;                                                                                    \
                ensureConditionEntriesMatchPlaceholders(#PlaceholderClass, map.keys(), p.placeholders());              \
                return false;                                                                                          \
            }();                                                                                                       \
            Q_UNUSED(testFlag)                                                                                         \
        } while (false)
#else
#    define MediaElch_Ensure_Condition_Matches_Placeholders(PlaceholderClass, conditionMap)                            \
        do {                                                                                                           \
        } while (false)
#endif


namespace mediaelch {

#ifdef QT_DEBUG
/// Test utility for asserting data / placeholder consistency.
/// Use via macro MediaElch_Ensure_Data_Matches_Placeholders
void ensureDataEntriesMatchPlaceholders(const char* variant,
    const QList<QString>& dataKeys,
    const QVector<Placeholder>& placeholders);
/// Test utility for asserting condition / placeholder consistency.
/// Use via macro MEDIAELCH_ENSURE_Condition_MATCHES_PLACEHOLDERS
void ensureConditionEntriesMatchPlaceholders(const char* variant,
    const QList<QString>& conditionKeys,
    const QVector<Placeholder>& placeholders);
#endif

} // namespace mediaelch
