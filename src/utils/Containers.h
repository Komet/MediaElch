#pragma once

// Some container helper functions.  Most are only necessary because
// newer features are only available is more recetn Qt versions.

#include <QList>
#include <QSet>

namespace mediaelch {

template<class T>
QSet<T> listToSet(const QList<T>& list)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    return list.toSet();
#else
    return QSet<T>(list.begin(), list.end());
#endif
}

template<class T>
QVector<T> setToVector(const QSet<T>& set)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    return QVector<T>::fromList(set.values());
#else
    return QVector<T>(set.begin(), set.end());
#endif
}

/// Split a string at the given delimiter and trim all entries.
/// Removes empty entries.
QStringList split_string_trimmed(const QString& str, const QString& delimiter);

} // namespace mediaelch
