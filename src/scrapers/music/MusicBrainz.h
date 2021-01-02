#pragma once

#include "globals/ScraperInfos.h"
#include "music/MusicBrainzId.h"
#include "network/NetworkManager.h"
#include "network/WebsiteCache.h"
#include "scrapers/ScraperError.h"

#include <QObject>
#include <QString>

class Album;
class Artist;

namespace mediaelch {
namespace scraper {

class MusicBrainzApi : public QObject
{
    Q_OBJECT
public:
    explicit MusicBrainzApi(QObject* parent = nullptr);
    ~MusicBrainzApi() override = default;

public:
    using ApiCallback = std::function<void(QString, ScraperError)>;

    void sendGetRequest(const Locale& locale, const QUrl& url, ApiCallback callback);
    void searchForArtist(const Locale& locale, const QString& query, ApiCallback callback);
    void searchForAlbum(const Locale& locale, const QString& query, ApiCallback callback);
    void searchForAlbumWithArtist(const Locale& locale,
        const QString& albumQuery,
        const QString& artistName,
        ApiCallback callback);

    void loadArtist(const Locale& locale, const MusicBrainzId& artistId, ApiCallback callback);
    void loadAlbum(const Locale& locale, const MusicBrainzId& albumId, ApiCallback callback);

private:
    network::NetworkManager m_network;
    WebsiteCache m_cache;
};

class MusicBrainz : public QObject
{
    Q_OBJECT
public:
    explicit MusicBrainz(QObject* parent = nullptr);
    ~MusicBrainz() override = default;

public:
    void parseAndAssignAlbum(const QString& xml, Album* album, QSet<MusicScraperInfo> infos);
    void parseAndAssignArtist(const QString& data, Artist* artist, QSet<MusicScraperInfo> infos);

private:
    QString replaceCommonHtmlTags(QString text) const;
};

} // namespace scraper
} // namespace mediaelch
