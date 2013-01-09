#ifndef MOVIEMULTISCRAPEDIALOG_H
#define MOVIEMULTISCRAPEDIALOG_H

#include <QDialog>
#include <QQueue>
#include "movies/Movie.h"

namespace Ui {
class MovieMultiScrapeDialog;
}

class MovieMultiScrapeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MovieMultiScrapeDialog(QWidget *parent = 0);
    ~MovieMultiScrapeDialog();
    static MovieMultiScrapeDialog *instance(QWidget *parent = 0);
    void setMovies(QList<Movie*> movies);

public slots:
    int exec();
    void reject();
    void accept();

private slots:
    void onStartScraping();
    void onScrapingFinished();
    void onSearchFinished(QList<ScraperSearchResult> results);
    void scrapeNext();
    void onProgress(Movie *movie, int current, int maximum);
    void onChkToggled();
    void onChkAllToggled();

private:
    Ui::MovieMultiScrapeDialog *ui;
    QList<Movie*> m_movies;
    QQueue<Movie*> m_queue;
    Movie *m_currentMovie;
    ScraperInterface *m_scraperInterface;
    bool m_isImdb;
    bool m_isTmdb;
    bool m_executed;
    QList<int> m_infosToLoad;
    void loadMovieData(Movie *movie, QString id);
    bool isExecuted();
};

#endif // MOVIEMULTISCRAPEDIALOG_H
