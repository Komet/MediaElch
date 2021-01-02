#pragma once

#include "data/Locale.h"
#include "globals/ScraperInfos.h"
#include "music/MusicBrainzId.h"
#include "music/TheAudioDbId.h"
#include "network/NetworkManager.h"
#include "network/WebsiteCache.h"
#include "scrapers/ScraperError.h"

#include <QJsonObject>
#include <QObject>
#include <QString>

class Artist;
class Album;

namespace mediaelch {
namespace scraper {

class TheAudioDbApi : public QObject
{
    Q_OBJECT
public:
    explicit TheAudioDbApi(QObject* parent = nullptr);
    ~TheAudioDbApi() override = default;

public:
    using ApiCallback = std::function<void(QString, ScraperError)>;

    void sendGetRequest(const Locale& locale, const QUrl& url, ApiCallback callback);

public:
    QUrl makeArtistUrl(const MusicBrainzId& artistId);
    QUrl makeArtistUrl(const TheAudioDbId& artistId);
    QUrl makeArtistDiscographyUrl(const MusicBrainzId& artistId);
    QUrl makeArtistDiscographyUrl(const TheAudioDbId& artistId);

private:
    network::NetworkManager m_network;
    WebsiteCache m_cache;
    QString m_tadbApiKey;
};

class TheAudioDb : public QObject
{
    Q_OBJECT
public:
    explicit TheAudioDb(QObject* parent = nullptr);
    ~TheAudioDb() override = default;

public:
    void parseAndAssignArtist(QJsonObject document, Artist* artist, QSet<MusicScraperInfo> infos, const QString& lang);
    void parseAndAssignAlbum(QJsonObject document, Album* album, QSet<MusicScraperInfo> infos, const QString& lang);
    void parseAndAssignArtistDiscography(QJsonObject document, Artist* artist, QSet<MusicScraperInfo> infos);
};

} // namespace scraper
} // namespace mediaelch
