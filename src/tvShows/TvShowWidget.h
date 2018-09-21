#ifndef TVSHOWWIDGET_H
#define TVSHOWWIDGET_H

#include "globals/Globals.h"

#include <QWidget>

namespace Ui {
class TvShowWidget;
}

class TvShow;
class TvShowEpisode;

/**
 * @brief The TvShowWidget class
 */
class TvShowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowWidget(QWidget *parent = nullptr);
    ~TvShowWidget() override;
    void updateInfo();

public slots:
    void onTvShowSelected(TvShow *show);
    void onSeasonSelected(TvShow *show, int season);
    void onEpisodeSelected(TvShowEpisode *episode);
    void onSetEnabledTrue(TvShow *show = nullptr, int season = -1);
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
