#pragma once

#include "network/NetworkManager.h"
#include "scrapers/concert/ConcertScraper.h"
#include "scrapers/tmdb/TmdbApi.h"
#include "settings/ScraperSettings.h"

#include <QComboBox>
#include <QLocale>
#include <QNetworkReply>
#include <QObject>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class TmdbConcert : public ConcertScraper
{
    Q_OBJECT
public:
    explicit TmdbConcert(QObject* parent = nullptr);
    ~TmdbConcert() override = default;

    static constexpr const char* ID = "TmdbConcert";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD ConcertSearchJob* search(ConcertSearchJob::Config config) override;

    void loadData(TmdbId id, Concert* concert, QSet<ConcertScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<ConcertScraperInfo> scraperSupports() override;
    QWidget* settingsWidget() override;

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
    QWidget* m_widget;
    QComboBox* m_box;

    void setup();
    QString localeForTMDb() const;
    QString language() const;
    QString country() const;
    mediaelch::network::NetworkManager* network();
    void parseAndAssignInfos(QString json, Concert* concert, QSet<ConcertScraperInfo> infos);
};

} // namespace scraper
} // namespace mediaelch
