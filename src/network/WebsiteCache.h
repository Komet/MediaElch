#pragma once

#include "data/Locale.h"

#include <QDateTime>
#include <QMap>
#include <QNetworkRequest>
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
    constexpr static int timeoutSeconds = 240; // 4min

    WebsiteCache();

    void addElement(const QNetworkRequest& request, QString data);
    QString getElement(const QNetworkRequest& request);
    bool hasValidElement(const QNetworkRequest& request);

    void clear();

private:
    struct CacheElement
    {
        QDateTime date;
        QString data;
    };

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
