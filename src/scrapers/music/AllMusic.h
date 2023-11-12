#pragma once

#include "data/AllMusicId.h"
#include "network/NetworkManager.h"
#include "scrapers/ScraperError.h"
#include "scrapers/ScraperInfos.h"

#include <QObject>
#include <QString>
#include <functional>

class Album;
class Artist;

namespace mediaelch {
namespace scraper {

class AllMusicApi : public QObject
{
    Q_OBJECT
public:
    explicit AllMusicApi(QObject* parent = nullptr);
    ~AllMusicApi() override = default;

public:
    using ApiCallback = std::function<void(QString, ScraperError)>;

public:
    QUrl makeArtistUrl(const AllMusicId& artistId);
    QUrl makeArtistBiographyUrl(const AllMusicId& artistId);
    QUrl makeArtistMoodsUrl(const AllMusicId& artistId);

    QUrl makeAlbumUrl(const AllMusicId& albumId);
    QUrl makeAlbumReviewUrl(const AllMusicId& albumId);
    QUrl makeAlbumMoodsUrl(const AllMusicId& albumId);

private:
    network::NetworkManager m_network;
};

class AllMusic : public QObject
{
    Q_OBJECT
public:
    explicit AllMusic(QObject* parent = nullptr);
    ~AllMusic() override = default;

public:
    void parseAndAssignAlbum(const QString& html, Album& album, const QSet<MusicScraperInfo>& infos);
    void parseAndAssignAlbumReview(const QString& html, Album& artist, const QSet<MusicScraperInfo>& infos);
    void parseAndAssignAlbumMoods(const QString& html, Album& artist, const QSet<MusicScraperInfo>& infos);
    void parseAndAssignArtist(const QString& html, Artist& artist, const QSet<MusicScraperInfo>& infos);
    void parseAndAssignArtistBiography(const QString& html, Artist& artist, const QSet<MusicScraperInfo>& infos);
    void parseAndAssignArtistMoods(const QString& html, Artist& artist, const QSet<MusicScraperInfo>& infos);
    void parseAndAssignArtistDiscography(const QString& html, Artist& artist, const QSet<MusicScraperInfo>& infos);
};

} // namespace scraper
} // namespace mediaelch
