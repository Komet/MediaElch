#pragma once

#include "data/Locale.h"

#include <QDateTime>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QUrl>

namespace mediaelch {
namespace network {

/// \brief TheTvDb API cache stores the result for a API request as a string.
///
/// Clears cache elements every timeoutSeconds. The cache is *not*
/// thread safe.
class WebsiteCache
{
public:
    constexpr static int timeoutSeconds = 240;

    WebsiteCache();

    void addElement(const QUrl& url, const Locale& locale, QString data);
    QString getElement(const QUrl& url, const Locale& locale);
    bool hasValidElement(const QUrl& url, const Locale& locale);

private:
    struct CacheElement
    {
        QDateTime date;
        QString data;
    };

    QString hash(const QUrl& url, const Locale& locale);

    /// \brief Clears old cache entries that are older than timeoutSeconds.
    ///
    /// Restarts the timer if the cache is not empty to ensure that all elements
    /// are eventually deleted.
    void clearOldCacheEntries();

    QMap<QUrl, CacheElement> m_cache;
    QTimer m_timer;
};

} // namespace network
} // namespace mediaelch
