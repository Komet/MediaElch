#pragma once

#include "data/TmdbId.h"
#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/tmdb/TmdbMovieScrapeJob.h"
#include "scrapers/tmdb/TmdbApi.h"

#include <QComboBox>
#include <QLocale>
#include <QMap>
#include <QMutex>
#include <QNetworkReply>
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
    void loadCollectionFinished();
    void loadCastsFinished();
    void loadTrailersFinished();
    void loadImagesFinished();
    void loadReleasesFinished();

private:
    TmdbApi m_api;
    TmdbMovieScrapeJob m_scrapeJob;
    ScraperMeta m_meta;
    mediaelch::network::NetworkManager m_network;
    QMutex m_mutex;
    QSet<MovieScraperInfo> m_scraperNativelySupports;
    QPointer<QWidget> m_widget;
    QComboBox* m_box;

    QString localeForTMDb() const;
    QString language() const;
    QString country() const;

    void parseAndAssignInfos(QString json, Movie* movie, QSet<MovieScraperInfo> infos);
    /// Load the given collection (TMDb id) and store the content in the movie.
    void loadCollection(Movie* movie, const TmdbId& collectionTmdbId);
};

} // namespace scraper
} // namespace mediaelch
