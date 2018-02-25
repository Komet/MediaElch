#ifndef TVSHOWSEARCHEPISODE_H
#define TVSHOWSEARCHEPISODE_H

#include <QTableWidgetItem>
#include <QWidget>

#include "globals/Globals.h"

namespace Ui {
class TvShowSearchEpisode;
}

class TvShowSearchEpisode : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowSearchEpisode(QWidget *parent = 0);
    ~TvShowSearchEpisode();
    QString scraperId();
    QList<int> infosToLoad();

public slots:
    void search(QString searchString, QString id);

signals:
    void sigResultClicked();

private slots:
    void onSearch();
    void onShowResults(QList<ScraperSearchResult> results);
    void onResultClicked(QTableWidgetItem *item);
    void onChkToggled();
    void onChkAllToggled();

private:
    Ui::TvShowSearchEpisode *ui;
    void clear();
    QString m_scraperId;
    QList<int> m_infosToLoad;
};

#endif // TVSHOWSEARCHEPISODE_H
