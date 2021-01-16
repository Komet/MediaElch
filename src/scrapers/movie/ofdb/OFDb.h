#pragma once

#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/ofdb/OfdbApi.h"

#include <QNetworkReply>
#include <QObject>


namespace mediaelch {
namespace scraper {

class OFDb : public MovieScraper
{
    Q_OBJECT
public:
    explicit OFDb(QObject* parent = nullptr);
    static constexpr const char* ID = "ofdb";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD MovieSearchJob* search(MovieSearchJob::Config config) override;

public:
    void loadData(QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
        Movie* movie,
        QSet<MovieScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;

    QSet<MovieScraperInfo> scraperNativelySupports() override;

    void changeLanguage(mediaelch::Locale locale) override;
    QWidget* settingsWidget() override;

private slots:
    void loadFinished();

private:
    ScraperMeta m_meta;
    OfdbApi m_api;
    mediaelch::network::NetworkManager m_network;

    mediaelch::network::NetworkManager* network();
    void parseAndAssignInfos(QString data, Movie* movie, QSet<MovieScraperInfo> infos);
};

} // namespace scraper
} // namespace mediaelch
