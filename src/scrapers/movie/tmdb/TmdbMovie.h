#pragma once

#include "scrapers/movie/MovieScraper.h"
#include "scrapers/tmdb/TmdbApi.h"

#include <QComboBox>
#include <QMap>
#include <QObject>
#include <QPointer>

namespace mediaelch {
namespace scraper {

class TmdbMovie final : public MovieScraper
{
    Q_OBJECT
public:
    explicit TmdbMovie(QObject* parent = nullptr);
    ~TmdbMovie() override;
    static constexpr const char* ID = "TMDb";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD MovieSearchJob* search(MovieSearchJob::Config config) override;
    ELCH_NODISCARD MovieScrapeJob* loadMovie(MovieScrapeJob::Config config) override;

public:
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<MovieScraperInfo> scraperNativelySupports() override;
    void changeLanguage(mediaelch::Locale locale) override;
    QWidget* settingsWidget() override;


private:
    TmdbApi m_api;
    ScraperMeta m_meta;

    QSet<MovieScraperInfo> m_scraperNativelySupports;
    QPointer<QWidget> m_widget;
    QComboBox* m_box;
};

} // namespace scraper
} // namespace mediaelch
