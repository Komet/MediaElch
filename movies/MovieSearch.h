#ifndef MOVIESEARCH_H
#define MOVIESEARCH_H

#include <QDialog>
#include <QTableWidgetItem>

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
    int scraperNo();
    QString scraperId();
    QList<int> infosToLoad();

private slots:
    void search();
    void showResults(QList<ScraperSearchResult> results);
    void resultClicked(QTableWidgetItem *item);
    void chkToggled();
    void chkAllToggled(bool toggled);

private:
    Ui::MovieSearch *ui;
    int m_scraperNo;
    QString m_scraperId;
    QList<int> m_infosToLoad;

    void clear();
    void setChkBoxesEnabled(QList<int> scraperSupports);
};

#endif // MOVIESEARCH_H
