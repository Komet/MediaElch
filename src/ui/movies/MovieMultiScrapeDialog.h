#pragma once

#include "movies/Movie.h"
#include "scrapers/movie/MovieScraper.h"

#include <QDialog>
#include <QPointer>
#include <QQueue>

namespace Ui {
class MovieMultiScrapeDialog;
}

class MovieMultiScrapeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MovieMultiScrapeDialog(QWidget* parent = nullptr);
    ~MovieMultiScrapeDialog() override;
    static MovieMultiScrapeDialog* instance(QWidget* parent = nullptr);
    void setMovies(QVector<Movie*> movies);

public slots:
    int exec() override;
    void reject() override;
    void accept() override;

private slots:
    void onStartScraping();
    void onScrapingFinished();
    void onSearchFinished(QVector<mediaelch::scraper::MovieSearchJob::Result> results);
    void onSearchFailed(ScraperSearchError error);
    void scrapeNext();
    void onProgress(Movie* movie, int current, int maximum);
    void onChkToggled();
    void onChkAllToggled();
    void setCheckBoxesEnabled();

private:
    Ui::MovieMultiScrapeDialog* ui = nullptr;
    QVector<Movie*> m_movies;
    QQueue<Movie*> m_queue;
    QPointer<Movie> m_currentMovie;
    mediaelch::scraper::MovieScraper* m_scraperInterface = nullptr;
    QHash<mediaelch::scraper::MovieScraper*, QString> m_currentIds;
    bool m_isImdb = false;
    bool m_isTmdb = false;
    bool m_executed = false;
    QVector<MovieScraperInfos> m_infosToLoad;

    void loadMovieData(Movie& movie, QString id);
    bool isExecuted();
};
