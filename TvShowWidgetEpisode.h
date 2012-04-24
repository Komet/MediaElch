#ifndef TVSHOWWIDGETEPISODE_H
#define TVSHOWWIDGETEPISODE_H

#include <QLabel>
#include <QWidget>
#include "data/TvShowEpisode.h"
#include "DownloadManager.h"

namespace Ui {
class TvShowWidgetEpisode;
}

class TvShowWidgetEpisode : public QWidget
{
    Q_OBJECT
    
public:
    explicit TvShowWidgetEpisode(QWidget *parent = 0);
    ~TvShowWidgetEpisode();
    void setEpisode(TvShowEpisode *episode);

public slots:
    void onSetEnabled(bool enabled);
    void onClear();
    void onSaveInformation();
    void onStartScraperSearch();

signals:
    void sigSetActionSearchEnabled(bool, MainWidgets);
    void sigSetActionSaveEnabled(bool, MainWidgets);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void onAddDirector();
    void onRemoveDirector();
    void onAddWriter();
    void onRemoveWriter();
    void onChooseThumbnail();
    void onPosterDownloadFinished(DownloadManagerElement elem);
    void onLoadDone();

private:
    Ui::TvShowWidgetEpisode *ui;
    TvShowEpisode *m_episode;
    QLabel *m_savingWidget;
    QMovie *m_loadingMovie;
    QImage m_chosenThumbnail;
    DownloadManager *m_posterDownloadManager;

    void updateEpisodeInfo();
};

#endif // TVSHOWWIDGETEPISODE_H
