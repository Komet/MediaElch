#pragma once

#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "globals/ScraperResult.h"
#include "tv_shows/TvDbId.h"

#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class TvShowSearchEpisode;
}

class TvShowSearchEpisode : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowSearchEpisode(QWidget* parent = nullptr);
    ~TvShowSearchEpisode() override;
    TvDbId scraperId();
    QSet<ShowScraperInfo> infosToLoad();

public slots:
    void search(QString searchString, TvDbId id);

signals:
    void sigResultClicked();

private slots:
    void onSearch();
    void onShowResults(QVector<ScraperSearchResult> results);
    void onResultClicked(QTableWidgetItem* item);
    void onChkToggled();
    void onChkAllToggled();

private:
    Ui::TvShowSearchEpisode* ui;
    void clear();
    TvDbId m_scraperId;
    QSet<ShowScraperInfo> m_infosToLoad;
};
