#ifndef TVSHOWWIDGET_H
#define TVSHOWWIDGET_H

#include <QWidget>
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"

namespace Ui {
class TvShowWidget;
}

/**
 * @brief The TvShowWidget class
 */
class TvShowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowWidget(QWidget *parent = 0);
    ~TvShowWidget();
    void updateInfo();

public slots:
    void onTvShowSelected(TvShow *show);
    void onSeasonSelected(TvShow *show, int season);
    void onEpisodeSelected(TvShowEpisode *episode);
    void onSetEnabledTrue(TvShow *show = 0, int season = -1);
    void onSetEnabledTrue(TvShowEpisode *episode);
    void onSetDisabledTrue();
    void onClear();
    void onSaveInformation();
    void onSaveAll();
    void onStartScraperSearch();
    void setBigWindow(bool bigWindow);

signals:
    void sigSetActionSearchEnabled(bool, MainWidgets);
    void sigSetActionSaveEnabled(bool, MainWidgets);
    void sigDownloadsStarted(QString, int);
    void sigDownloadsProgress(int, int, int);
    void sigDownloadsFinished(int);

private:
    Ui::TvShowWidget *ui;
};

#endif // TVSHOWWIDGET_H
