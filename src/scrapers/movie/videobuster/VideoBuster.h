#pragma once

#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/videobuster/VideoBusterApi.h"

#include <QObject>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class VideoBuster : public MovieScraper
{
    Q_OBJECT
public:
    explicit VideoBuster(QObject* parent = nullptr);
    static constexpr const char* ID = "videobuster";

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

private:
    ScraperMeta m_meta;
    VideoBusterApi m_api;

private:
    void parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfo> infos);
    QString replaceEntities(const QString msg);
};

} // namespace scraper
} // namespace mediaelch
