#pragma once

#include "globals/ScraperInfos.h"
#include "music/AllMusicId.h"
#include "network/NetworkManager.h"
#include "network/WebsiteCache.h"
#include "scrapers/ScraperError.h"

#include <QObject>
#include <QString>

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

private:
    network::NetworkManager m_network;
    WebsiteCache m_cache;
};

class AllMusic : public QObject
{
    Q_OBJECT
public:
    explicit AllMusic(QObject* parent = nullptr);
    ~AllMusic() override = default;

public:
    void parseAndAssignAlbum(const QString& html, Album* album, QSet<MusicScraperInfo> infos);
    void parseAndAssignArtist(const QString& html, Artist* artist, QSet<MusicScraperInfo> infos);
    void parseAndAssignArtistBiography(const QString& html, Artist* artist, QSet<MusicScraperInfo> infos);
    void parseAndAssignArtistDiscography(const QString& html, Artist* artist, QSet<MusicScraperInfo> infos);

private:
    QString trim(QString text);
};

} // namespace scraper
} // namespace mediaelch
