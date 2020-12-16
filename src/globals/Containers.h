#pragma once

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

} // namespace mediaelch
