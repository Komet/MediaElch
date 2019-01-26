#pragma once

#include "data/TvDbId.h"
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
    TvDbId scraperId();
    QList<TvShowScraperInfos> infosToLoad();

public slots:
    void search(QString searchString, TvDbId id);

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
    TvDbId m_scraperId;
    QList<TvShowScraperInfos> m_infosToLoad;
};
