#ifndef TVSHOWSEARCHEPISODE_H
#define TVSHOWSEARCHEPISODE_H

#include "globals/Globals.h"

#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class TvShowSearchEpisode;
}

class TvShowSearchEpisode : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowSearchEpisode(QWidget *parent = nullptr);
    ~TvShowSearchEpisode() override;
    QString scraperId();
    QList<TvShowScraperInfos> infosToLoad();

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
    QList<TvShowScraperInfos> m_infosToLoad;
};

#endif // TVSHOWSEARCHEPISODE_H
