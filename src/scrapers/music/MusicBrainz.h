#pragma once

#include "data/AllMusicId.h"
#include "data/MusicBrainzId.h"
#include "network/NetworkManager.h"
#include "scrapers/ScraperError.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/ScraperResult.h"

#include <QObject>
#include <QString>
#include <functional>

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
    void loadReleaseGroup(const Locale& locale, const MusicBrainzId& groupId, MusicBrainzApi::ApiCallback callback);

    QVector<ScraperSearchResult> parseArtistSearchPage(const QString& html);
    QVector<ScraperSearchResult> parseAlbumSearchPage(const QString& html);

    QUrl makeArtistSearchUrl(const QString& query);
    QUrl makeAlbumSearchUrl(const QString& query);
    QUrl makeAlbumWithArtistSearchUrl(const QString& albumQuery, const QString& artistName);
    QUrl makeArtistBiographyUrl(const MusicBrainzId& artistId);

private:
    network::NetworkManager m_network;
};

class MusicBrainz : public QObject
{
    Q_OBJECT
public:
    explicit MusicBrainz(QObject* parent = nullptr);
    ~MusicBrainz() override = default;

public:
    void parseAndAssignAlbum(const QString& xml, Album& album, const QSet<MusicScraperInfo>& infos);
    void parseAndAssignArtist(const QString& data, Artist& artist, const QSet<MusicScraperInfo>& infos);

public:
    static QPair<AllMusicId, QString> extractAllMusicIdAndDiscogsUrl(const QString& xml);
};

} // namespace scraper
} // namespace mediaelch
