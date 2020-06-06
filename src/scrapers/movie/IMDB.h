#pragma once

#include "movies/Movie.h"
#include "scrapers/movie/MovieScraperInterface.h"

#include <QMutexLocker>
#include <QNetworkAccessManager>
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
    void loadData(QHash<MovieScraperInterface*, QString> ids, Movie* movie, QSet<MovieScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<MovieScraperInfos> scraperSupports() override;
    QSet<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    void changeLanguage(QString languageKey) override;
    QString defaultLanguageKey() override;
    QWidget* settingsWidget() override;
    bool isAdult() const override;
    void parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfos> infos);

private slots:
    void onSearchFinished();
    void onSearchIdFinished();
    void onSearchFinishedAdult();
    void onLoadDone(Movie& movie, ImdbMovieLoader* loader);

private:
    QWidget* m_settingsWidget;
    QCheckBox* m_loadAllTagsWidget;

    bool m_loadAllTags = false;
    QNetworkAccessManager m_qnam;
    QSet<MovieScraperInfos> m_scraperSupports;

    QVector<ScraperSearchResult> parseSearch(QString html);
    QVector<ScraperSearchResult> parseSearchAdult(QString html);
};
