#pragma once

#include "data/Locale.h"
#include "data/MusicBrainzId.h"
#include "data/TheAudioDbId.h"
#include "network/NetworkManager.h"
#include "scrapers/ScraperError.h"
#include "scrapers/ScraperInfos.h"

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <functional>

class Artist;
class Album;

namespace mediaelch {
namespace scraper {

// TODO: Get new API key, seems as if the old was deactivated.
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
    // TODO: Remove, paid API
    QUrl makeArtistUrl(const MusicBrainzId& artistId);
    QUrl makeArtistUrl(const TheAudioDbId& artistId);
    // TODO: Remove, paid API
    QUrl makeArtistDiscographyUrl(const MusicBrainzId& artistId);
    QUrl makeArtistDiscographyUrl(const TheAudioDbId& artistId);

private:
    network::NetworkManager m_network;
    QString m_tadbApiKey;
};

class TheAudioDb : public QObject
{
    Q_OBJECT
public:
    explicit TheAudioDb(QObject* parent = nullptr);
    ~TheAudioDb() override = default;

public:
    void parseAndAssignArtist(QJsonObject document,
        Artist& artist,
        const QSet<MusicScraperInfo>& infos,
        const QString& lang);
    void
    parseAndAssignAlbum(QJsonObject document, Album& album, const QSet<MusicScraperInfo>& infos, const QString& lang);
    void parseAndAssignArtistDiscography(QJsonObject document, Artist& artist, const QSet<MusicScraperInfo>& infos);
};

} // namespace scraper
} // namespace mediaelch
