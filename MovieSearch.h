#ifndef MOVIESEARCH_H
#define MOVIESEARCH_H

#include <QDialog>
#include <QTableWidgetItem>

#include "Globals.h"

namespace Ui {
class MovieSearch;
}

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

private slots:
    void search();
    void showResults(QList<ScraperSearchResult> results);
    void resultClicked(QTableWidgetItem *item);

private:
    Ui::MovieSearch *ui;
    void clear();

    int m_scraperNo;
    QString m_scraperId;
};

#endif // MOVIESEARCH_H
