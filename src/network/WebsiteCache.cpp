#include "network/WebsiteCache.h"

#include <QDateTime>
#include <QString>
#include <QUrl>

namespace mediaelch {
namespace scraper {

WebsiteCache::WebsiteCache()
{
    QObject::connect(&m_timer, &QTimer::timeout, &m_timer, [this]() { clearOldCacheEntries(); });
}

bool WebsiteCache::hasValidElement(const QUrl& url, const Locale& locale)
{
    const QString h = hash(url, locale);
    return m_cache.contains(h) && m_cache[h].date >= QDateTime::currentDateTime().addSecs(-timeoutSeconds);
}

QString WebsiteCache::hash(const QUrl& url, const Locale& locale)
{
    return QStringLiteral("%1_##_%2").arg(locale.toString(), url.toString());
}

void WebsiteCache::addElement(const QUrl& url, const Locale& locale, QString data)
{
    if (data.isEmpty() || !url.isValid()) {
        return;
    }
    CacheElement c;
    c.data = std::move(data);
    c.date = QDateTime::currentDateTime();
    m_cache.insert(hash(url, locale), c);

    if (!m_timer.isActive()) {
        // set timer for clearing the cache
        m_timer.start(timeoutSeconds * 1000);
    }
}

QString WebsiteCache::getElement(const QUrl& url, const Locale& locale)
{
    const QString h = hash(url, locale);
    return m_cache[h].data;
}

void WebsiteCache::clearOldCacheEntries()
{
    auto it = m_cache.begin();
    while (it != m_cache.end()) {
        if (it.value().date >= QDateTime::currentDateTime().addSecs(-timeoutSeconds)) {
            it = m_cache.erase(it);
        } else {
            ++it;
        }
    }

    if (!m_cache.isEmpty()) {
        m_timer.start(timeoutSeconds * 1000);
    }
}

} // namespace scraper
} // namespace mediaelch
