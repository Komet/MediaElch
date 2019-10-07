#pragma once

#include <QDateTime>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QUrl>

namespace thetvdb {

/// TheTvDb API cache stores the result for a API request as a string.
/// Clears cache elements every timeoutSeconds. The cache is *not*
/// thread safe.
class Cache
{
public:
    static Cache& instance();

    constexpr static int timeoutSeconds = 240;

    void addElement(QUrl url, QString data);
    QString getElement(QUrl url);
    bool containsValidElement(QUrl url);

private:
    struct CacheElement
    {
        QDateTime date;
        QString data;
    };

    Cache();
    void clearOldCacheEntries();

    QMap<QUrl, CacheElement> m_cache;
    QTimer m_timer;
};

void addCacheElement(QUrl url, QString data);
QString getCacheElement(QUrl url);
bool hasValidCacheElement(QUrl url);

} // namespace thetvdb
