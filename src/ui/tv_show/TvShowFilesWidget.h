#pragma once

#include "globals/Filter.h"
#include "globals/Globals.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"
#include "tv_shows/TvShowProxyModel.h"

#include <QAction>
#include <QMenu>
#include <QModelIndex>
#include <QWidget>
#include <functional>

namespace Ui {
class TvShowFilesWidget;
}

class TvShowBaseModelItem;

/// The TvShowFilesWidget class is responsible for showing a list of TV shows
/// with correct sorting and filtering. Internally, a TvShowTreeView is used
/// to display the TV shows with their seasons and episodes.
class TvShowFilesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowFilesWidget(QWidget* parent = nullptr);
    ~TvShowFilesWidget() override;

    void setFilter(const QVector<Filter*>& filters, QString text);
    static TvShowFilesWidget& instance();
    QVector<TvShowEpisode*> selectedEpisodes(bool includeFromSeasonOrShow = true);
    QVector<TvShow*> selectedShows();
    QVector<TvShow*> selectedSeasons();

public slots:
    void renewModel(bool force = false);
    void emitLastSelection();
    void multiScrape();
    // void updateProxy();

signals:
    void sigEpisodeSelected(TvShowEpisode* episode);
    void sigTvShowSelected(TvShow* show);
    void sigSeasonSelected(TvShow* show, SeasonNumber season);
    void sigNothingSelected();
    void sigStartSearch();

private slots:
    void onItemSelected(const QModelIndex& current, const QModelIndex& previous);
    void showContextMenu(QPoint point);
    void scanForEpisodes();
    void markAsWatched();
    void markAsUnwatched();
    void loadStreamDetails();
    void markForSyncBool(bool markForSync);
    void markForSync();
    void unmarkForSync();
    void openFolder();
    void openNfo();
    void showMissingEpisodes();
    void hideSpecialsInMissingEpisodes();
    void updateStatusLabel();
    void playEpisode(QModelIndex idx);

private:
    void setupContextMenu();
    void emitSelected(QModelIndex proxyIndex);
    void forEachSelectedItem(std::function<void(TvShowBaseModelItem&)> callback);

    static TvShowFilesWidget* m_instance;

    Ui::TvShowFilesWidget* ui = nullptr;
    TvShowProxyModel* m_tvShowProxyModel = nullptr;
    QMenu* m_contextMenu = nullptr;

    TvShowBaseModelItem* m_lastItem = nullptr;

    QAction* m_actionShowMissingEpisodes = nullptr;
    QAction* m_actionHideSpecialsInMissingEpisodes = nullptr;
};
