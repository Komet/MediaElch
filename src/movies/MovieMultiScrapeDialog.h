#pragma once

#include <QDialog>
#include <QPointer>
#include <QQueue>

#include "data/Movie.h"

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
    void onSearchFinished(QVector<ScraperSearchResult> results);
    void scrapeNext();
    void onProgress(Movie* movie, int current, int maximum);
    void onChkToggled();
    void onChkAllToggled();
    void setCheckBoxesEnabled();

private:
    Ui::MovieMultiScrapeDialog* ui;
    QVector<Movie*> m_movies;
    QQueue<Movie*> m_queue;
    QPointer<Movie> m_currentMovie;
    MovieScraperInterface* m_scraperInterface;
    QMap<MovieScraperInterface*, QString> m_currentIds;
    bool m_isImdb;
    bool m_isTmdb;
    bool m_executed;
    QVector<MovieScraperInfos> m_infosToLoad;
    void loadMovieData(Movie* movie, ImdbId id);
    void loadMovieData(Movie* movie, TmdbId id);
    bool isExecuted();
};
