#pragma once

#include "RenamerPlaceholders.h"

#include <QDir>
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

/// \brief   Returns a directory name based on \p desiredName that does not yet
///          exist inside \p dir.
/// \details If \p dir has no entry called \p desiredName, it is returned as-is.
///          Otherwise " 1", " 2", … is appended to \p desiredName (always to
///          \p desiredName itself, never to an already-suffixed result) until a
///          free name is found.
QString uniqueDirectoryName(const QDir& dir, const QString& desiredName);

#ifdef QT_DEBUG
/// Test utility for asserting data / placeholder consistency.
/// Use via macro MediaElch_Ensure_Data_Matches_Placeholders
void ensureDataEntriesMatchPlaceholders(const char* variant,
    const QList<QString>& dataKeys,
    const QVector<Placeholder>& placeholders);
/// Test utility for asserting condition / placeholder consistency.
/// Use via macro MediaElch_Ensure_Condition_Matches_Placeholders
void ensureConditionEntriesMatchPlaceholders(const char* variant,
    const QList<QString>& conditionKeys,
    const QVector<Placeholder>& placeholders);
#endif

} // namespace mediaelch
