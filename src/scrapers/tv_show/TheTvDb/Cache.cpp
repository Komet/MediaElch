#include "Cache.h"

#include <QDateTime>
#include <QString>
#include <QUrl>

namespace thetvdb {

void addCacheElement(QUrl url, QString data)
{
    Cache::instance().addElement(url, data);
}

QString getCacheElement(QUrl url)
{
    return Cache::instance().getElement(url);
}

bool hasValidCacheElement(QUrl url)
{
    return Cache::instance().containsValidElement(url);
}

Cache& Cache::instance()
{
    static Cache cache;
    return cache;
}

Cache::Cache()
{
    QObject::connect(&m_timer, &QTimer::timeout, [this]() { clearOldCacheEntries(); });
}

bool Cache::containsValidElement(QUrl url)
{
    return m_cache.contains(url) && m_cache[url].date >= QDateTime::currentDateTime().addSecs(-timeoutSeconds);
}

void Cache::addElement(QUrl url, QString data)
{
    if (data.isEmpty() || !url.isValid()) {
        return;
    }

    CacheElement c;
    c.data = std::move(data);
    c.date = QDateTime::currentDateTime();
    m_cache.insert(url, c);

    if (!m_timer.isActive()) {
        // set timer for clearing the cache
        m_timer.start(timeoutSeconds * 1000);
    }
}

QString Cache::getElement(QUrl url)
{
    return m_cache[url].data;
}

/// Clears old cache entries that are older than timeoutSeconds. Restarts
/// the timer if the cache is not empty to ensure that all elements are
/// eventually deleted.
void Cache::clearOldCacheEntries()
{
    auto it = m_cache.begin();
    while (it != m_cache.end()) {
        if (it.value().date >= QDateTime::currentDateTime().addSecs(-timeoutSeconds)) {
            it = m_cache.erase(it);
        } else {
            ++it;
        }
    }

    if (!m_cache.empty()) {
        m_timer.start(timeoutSeconds * 1000);
    }
}

} // namespace thetvdb
