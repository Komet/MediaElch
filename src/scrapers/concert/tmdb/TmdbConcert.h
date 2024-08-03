#pragma once

#include "network/NetworkManager.h"
#include "scrapers/concert/ConcertScraper.h"
#include "scrapers/tmdb/TmdbApi.h"

#include <QLocale>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>

namespace mediaelch {
namespace scraper {

class TmdbConcert : public ConcertScraper
{
    Q_OBJECT
public:
    explicit TmdbConcert(QObject* parent = nullptr);
    ~TmdbConcert() override;

    static constexpr const char* ID = "TmdbConcert";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD ConcertSearchJob* search(ConcertSearchJob::Config config) override;

    void loadData(TmdbId id, Concert* concert, QSet<ConcertScraperInfo> infos) override;


private slots:
    void loadFinished();
    void loadTrailersFinished();
    void loadImagesFinished();
    void loadReleasesFinished();
    void setupFinished();

private:
    ScraperMeta m_meta;
    TmdbApi m_api;

    QString m_apiKey;
    mediaelch::network::NetworkManager m_network;
    QLocale m_locale;
    QString m_language2;
    QString m_baseUrl;

    void setup();
    QString localeForTmdb() const;
    QString language() const;
    QString country() const;
    mediaelch::network::NetworkManager* network();
    void parseAndAssignInfos(QString json, Concert* concert, QSet<ConcertScraperInfo> infos);
};

} // namespace scraper
} // namespace mediaelch
