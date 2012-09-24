#ifndef CONCERTSEARCH_H
#define CONCERTSEARCH_H

#include <QDialog>
#include <QTableWidgetItem>

#include "globals/Globals.h"

namespace Ui {
class ConcertSearch;
}

/**
 * @brief The ConcertSearch class
 */
class ConcertSearch : public QDialog
{
    Q_OBJECT
public:
    explicit ConcertSearch(QWidget *parent = 0);
    ~ConcertSearch();

public slots:
    int exec(QString searchString);
    static ConcertSearch *instance(QWidget *parent = 0);
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
    Ui::ConcertSearch *ui;
    int m_scraperNo;
    QString m_scraperId;
    QList<int> m_infosToLoad;

    void clear();
    void setChkBoxesEnabled(QList<int> scraperSupports);
};

#endif // CONCERTSEARCH_H
