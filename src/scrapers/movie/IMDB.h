#pragma once

#include "movies/Movie.h"
#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraperInterface.h"

#include <QMutexLocker>
#include <QNetworkReply>

class QCheckBox;

class ImdbMovieLoader;

class IMDB : public MovieScraperInterface
{
    Q_OBJECT
public:
    explicit IMDB(QObject* parent = nullptr);
    static constexpr const char* scraperIdentifier = "IMDb";

    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    /// Load a movie for the given details.
    /// Due to the limited scraper API, we load a lot of data sequentially.
    ///   1. Basic Details
    ///   2 .(optional) Poster in higher resolution
    ///   3. (optional) Load Tags
    void loadData(QHash<MovieScraperInterface*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<MovieScraperInfo> scraperSupports() override;
    QSet<MovieScraperInfo> scraperNativelySupports() override;
    QVector<mediaelch::Locale> supportedLanguages() override;
    void changeLanguage(mediaelch::Locale locale) override;
    mediaelch::Locale defaultLanguage() override;
    QWidget* settingsWidget() override;
    bool isAdult() const override;
    void parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfo> infos);

private slots:
    void onSearchFinished();
    void onSearchIdFinished();
    void onLoadDone(Movie& movie, ImdbMovieLoader* loader);

private:
    QWidget* m_settingsWidget;
    QCheckBox* m_loadAllTagsWidget;

    bool m_loadAllTags = false;
    mediaelch::network::NetworkManager m_network;
    QSet<MovieScraperInfo> m_scraperSupports;

    QVector<ScraperSearchResult> parseSearch(const QString& html);
};
