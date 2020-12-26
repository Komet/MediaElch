#pragma once

#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/hotmovies/HotMoviesApi.h"

#include <QComboBox>
#include <QObject>
#include <QWidget>


namespace mediaelch {
namespace scraper {

class HotMovies : public MovieScraper
{
    Q_OBJECT
public:
    explicit HotMovies(QObject* parent = nullptr);
    static constexpr const char* ID = "hotmovies";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

public:
    void search(QString searchStr) override;
    void loadData(QHash<MovieScraper*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;

    QSet<MovieScraperInfo> scraperNativelySupports() override;

    void changeLanguage(mediaelch::Locale locale) override;
    QWidget* settingsWidget() override;

private:
    ScraperMeta m_meta;
    HotMoviesApi m_api;
    mediaelch::network::NetworkManager m_network;

private:
    mediaelch::network::NetworkManager* network();
    QVector<ScraperSearchResult> parseSearch(QString html);
    void parseAndAssignInfos(QString html, Movie* movie, QSet<MovieScraperInfo> infos);
};

} // namespace scraper
} // namespace mediaelch
