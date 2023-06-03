#include "network/WebsiteCache.h"

#include <QDateTime>
#include <QDebug>
#include <QString>
#include <QUrl>

namespace {

/// \brief Returns a hash based on the given network request's URL and headers.
QString hashFor(const QNetworkRequest& request)
{
    QString hash = request.url().toEncoded(QUrl::FullyEncoded);
    hash += " | ";

    QList<QByteArray> headers = request.rawHeaderList();
    for (const QByteArray& header : headers) {
        // "User-Agent" is not relevant and would bloat the hash, which is already huge.
        if (header != "User-Agent") {
            hash += header;
            hash += ": ";
            hash += request.rawHeader(header);
            hash += '\n';
        }
    }

    return hash;
}

} // namespace

namespace mediaelch {
namespace network {

WebsiteCache::WebsiteCache()
{
    QObject::connect(&m_timer, &QTimer::timeout, &m_timer, [this]() { clearOldCacheEntries(); });
}

bool WebsiteCache::hasValidElement(const QNetworkRequest& request)
{
    const QString h = hashFor(request);
    return m_cache.contains(h) && m_cache[h].date >= QDateTime::currentDateTime().addSecs(-timeoutSeconds);
}

void WebsiteCache::clear()
{
    m_cache.clear();
}

void WebsiteCache::addElement(const QNetworkRequest& request, QString data)
{
    if (data.isEmpty() || !request.url().isValid()) {
        return;
    }
    CacheElement c;
    c.data = std::move(data);
    c.date = QDateTime::currentDateTime();
    m_cache.insert(hashFor(request), c);

    if (!m_timer.isActive()) {
        // set timer for clearing the cache
        m_timer.start(timeoutSeconds * 1000);
    }
}

QString WebsiteCache::getElement(const QNetworkRequest& request)
{
    const QString h = hashFor(request);
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

} // namespace network
} // namespace mediaelch
