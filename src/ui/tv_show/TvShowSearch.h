#pragma once

#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "globals/ScraperResult.h"
#include "tv_shows/TvDbId.h"

#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class TvShowSearch;
}

class TvShowSearch : public QDialog
{
    Q_OBJECT

public:
    explicit TvShowSearch(QWidget* parent = nullptr);
    ~TvShowSearch() override;
    TvDbId scraperId();
    QSet<ShowScraperInfo> infosToLoad();
    void setSearchType(TvShowType type);
    TvShowUpdateType updateType();

public slots:
    int exec() override;
    int exec(QString searchString, TvDbId id);
    static TvShowSearch* instance(QWidget* parent = nullptr);

private slots:
    void onSearch();
    void onShowResults(QVector<ScraperSearchResult> results);
    void onResultClicked(QTableWidgetItem* item);
    void onShowInfoToggled();
    void onChkAllToggled();
    void onUpdateTypeChanged(int index);
    void onSeasonOrderChanged(int index);

private:
    void setupSeasonOrderComboBox();

private:
    Ui::TvShowSearch* ui = nullptr;
    void clear();
    TvDbId m_scraperId;
    QSet<ShowScraperInfo> m_infosToLoad;
    TvShowType m_searchType = TvShowType::None;
};
