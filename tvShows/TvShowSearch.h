#ifndef TVSHOWSEARCH_H
#define TVSHOWSEARCH_H

#include <QDialog>
#include <QTableWidgetItem>
#include "globals/Globals.h"

namespace Ui {
class TvShowSearch;
}

/**
 * @brief The TvShowSearch class
 */
class TvShowSearch : public QDialog
{
    Q_OBJECT

public:
    explicit TvShowSearch(QWidget *parent = 0);
    ~TvShowSearch();
    QString scraperId();

public slots:
    int exec(QString searchString);
    static TvShowSearch *instance(QWidget *parent = 0);
    void setChkUpdateAllVisible(bool visible);
    bool updateAll();

private slots:
    void onSearch();
    void onShowResults(QList<ScraperSearchResult> results);
    void onResultClicked(QTableWidgetItem *item);

private:
    Ui::TvShowSearch *ui;
    void clear();
    QString m_scraperId;
};

#endif // TVSHOWSEARCH_H
