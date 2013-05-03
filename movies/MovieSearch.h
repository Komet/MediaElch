#ifndef MOVIESEARCH_H
#define MOVIESEARCH_H

#include <QDialog>
#include <QTableWidgetItem>
#include "data/ScraperInterface.h"
#include "globals/Globals.h"

namespace Ui {
class MovieSearch;
}

/**
 * @brief The MovieSearch class
 */
class MovieSearch : public QDialog
{
    Q_OBJECT
public:
    explicit MovieSearch(QWidget *parent = 0);
    ~MovieSearch();

public slots:
    int exec(QString searchString);
    static MovieSearch *instance(QWidget *parent = 0);
    QString scraperId();
    QString scraperMovieId();
    QList<int> infosToLoad();
    QMap<ScraperInterface*, QString> customScraperIds();

private slots:
    void search();
    void showResults(QList<ScraperSearchResult> results);
    void resultClicked(QTableWidgetItem *item);
    void chkToggled();
    void chkAllToggled(bool toggled);

private:
    Ui::MovieSearch *ui;
    QString m_scraperId;
    QString m_scraperMovieId;
    QList<int> m_infosToLoad;
    QMap<ScraperInterface*, QString> m_customScraperIds;
    ScraperInterface *m_currentCustomScraper;

    void clear();
    void setChkBoxesEnabled(QList<int> scraperSupports);
};

#endif // MOVIESEARCH_H
