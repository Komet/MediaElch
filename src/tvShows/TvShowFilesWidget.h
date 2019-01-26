#pragma once

#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "data/TvShowProxyModel.h"
#include "globals/Filter.h"
#include "globals/Globals.h"

#include <QAction>
#include <QMenu>
#include <QModelIndex>
#include <QWidget>

namespace Ui {
class TvShowFilesWidget;
}

/**
 * @brief The TvShowFilesWidget class
 */
class TvShowFilesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowFilesWidget(QWidget *parent = nullptr);
    ~TvShowFilesWidget() override;
    void setFilter(QVector<Filter *> filters, QString text);
    static TvShowFilesWidget *instance();
    QVector<TvShowEpisode *> selectedEpisodes(bool includeFromSeasonOrShow = true);
    QVector<TvShow *> selectedShows();
    QVector<TvShow *> selectedSeasons();

public slots:
    void renewModel(bool force = false);
    void emitLastSelection();
    void multiScrape();
    void updateProxy();

signals:
    void sigEpisodeSelected(TvShowEpisode *episode);
    void sigTvShowSelected(TvShow *show);
    void sigSeasonSelected(TvShow *show, SeasonNumber season);
    void sigNothingSelected();
    void sigStartSearch();

private slots:
    void onItemSelected(QModelIndex index);
    void showContextMenu(QPoint point);
    void scanForEpisodes();
    void markAsWatched();
    void markAsUnwatched();
    void loadStreamDetails();
    void markForSync();
    void unmarkForSync();
    void openFolder();
    void openNfo();
    void showMissingEpisodes();
    void hideSpecialsInMissingEpisodes();
    void onViewUpdated();
    void playEpisode(QModelIndex idx);

private:
    Ui::TvShowFilesWidget *ui;
    TvShowProxyModel *m_tvShowProxyModel;
    static TvShowFilesWidget *m_instance;
    QMenu *m_contextMenu;
    TvShow *m_lastTvShow;
    TvShowEpisode *m_lastEpisode;
    SeasonNumber m_lastSeason;
    QAction *m_actionShowMissingEpisodes;
    QAction *m_actionHideSpecialsInMissingEpisodes;
};
