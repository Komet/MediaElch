#pragma once

#include "data/TvDbId.h"
#include "globals/Globals.h"

#include <QDialog>
#include <QTableWidgetItem>

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
    explicit TvShowSearch(QWidget* parent = nullptr);
    ~TvShowSearch() override;
    TvDbId scraperId();
    QVector<TvShowScraperInfos> infosToLoad();
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
    void onChkToggled();
    void onChkAllToggled();
    void onComboIndexChanged();
    void onChkDvdOrderToggled();

private:
    Ui::TvShowSearch* ui;
    void clear();
    TvDbId m_scraperId;
    QVector<TvShowScraperInfos> m_infosToLoad;
    TvShowType m_searchType;
};
