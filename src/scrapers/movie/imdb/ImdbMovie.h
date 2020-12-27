#pragma once

#include "movies/Movie.h"
#include "network/NetworkManager.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/movie/MovieScraper.h"

#include <QMutexLocker>
#include <QNetworkReply>

class QCheckBox;

namespace mediaelch {
namespace scraper {

class ImdbMovieLoader;

class ImdbMovie : public MovieScraper
{
    Q_OBJECT
public:
    explicit ImdbMovie(QObject* parent = nullptr);
    static constexpr const char* ID = "IMDb";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    void search(QString searchStr) override;
    /// Load a movie for the given details.
    /// Due to the limited scraper API, we load a lot of data sequentially.
    ///   1. Basic Details
    ///   2 .(optional) Poster in higher resolution
    ///   3. (optional) Load Tags
    void loadData(QHash<MovieScraper*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;

    QSet<MovieScraperInfo> scraperNativelySupports() override;
    void changeLanguage(mediaelch::Locale locale) override;
    QWidget* settingsWidget() override;
    void parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfo> infos) const;

private slots:
    void onLoadDone(Movie& movie, ImdbMovieLoader* loader);

private:
    ImdbApi m_api;
    ScraperMeta m_meta;
    QWidget* m_settingsWidget;
    QCheckBox* m_loadAllTagsWidget;

    bool m_loadAllTags = false;
    mediaelch::network::NetworkManager m_network;

    QVector<ScraperSearchResult> parseSearch(const QString& html);
    ScraperSearchResult parseIdFromMovieHtml(const QString& html);
};

} // namespace scraper
} // namespace mediaelch
