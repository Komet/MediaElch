#ifndef TVSHOWWIDGETSEASON_H
#define TVSHOWWIDGETSEASON_H

#include <QLabel>
#include <QMovie>
#include <QResizeEvent>
#include <QWidget>
#include "data/TvShow.h"
#include "globals/DownloadManager.h"
#include "globals/Globals.h"

namespace Ui {
class TvShowWidgetSeason;
}

class TvShowWidgetSeason : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowWidgetSeason(QWidget *parent = 0);
    ~TvShowWidgetSeason();
    void setSeason(TvShow *show, int season);

public slots:
    void onClear();
    void onSaveInformation();
    void onSetEnabled(bool enabled);

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void sigSetActionSearchEnabled(bool, MainWidgets);
    void sigSetActionSaveEnabled(bool, MainWidgets);

private slots:
    void onChoosePoster();
    void onChooseBackdrop();
    void onChooseBanner();
    void onDeletePoster();
    void onDeleteBackdrop();
    void onDeleteBanner();
    void onRevertChanges();
    void onDownloadFinished(DownloadManagerElement elem);

private:
    Ui::TvShowWidgetSeason *ui;
    TvShow *m_show;
    int m_season;
    QLabel *m_savingWidget;
    QMovie *m_loadingMovie;
    DownloadManager *m_downloadManager;
    void updateImages(QList<ImageType> images);
};

#endif // TVSHOWWIDGETSEASON_H
