#ifndef TVSHOWWIDGET_H
#define TVSHOWWIDGET_H

#include <QWidget>
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"

namespace Ui {
class TvShowWidget;
}

class TvShowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowWidget(QWidget *parent = 0);
    ~TvShowWidget();

public slots:
    void onTvShowSelected(TvShow *show);
    void onEpisodeSelected(TvShowEpisode *episode);
    void onSetEnabledTrue(TvShow *show = 0);
    void onSetEnabledTrue(TvShowEpisode *episode);
    void onSetDisabledTrue();
    void onClear();
    void onSaveInformation();
    void onSaveAll();
    void onStartScraperSearch();

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
