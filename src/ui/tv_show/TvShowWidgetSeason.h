#pragma once

#include "globals/DownloadManager.h"
#include "globals/Globals.h"
#include "tv_shows/TvShow.h"

#include <QLabel>
#include <QMovie>
#include <QResizeEvent>
#include <QWidget>

namespace Ui {
class TvShowWidgetSeason;
}

class TvShowWidgetSeason : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowWidgetSeason(QWidget* parent = nullptr);
    ~TvShowWidgetSeason() override;
    void setSeason(TvShow* show, SeasonNumber season);
    void updateSeasonInfo();

public slots:
    void onClear();
    void onSaveInformation();
    void onSetEnabled(bool enabled);

protected:
    void resizeEvent(QResizeEvent* event) override;

signals:
    void sigSetActionSearchEnabled(bool, MainWidgets);
    void sigSetActionSaveEnabled(bool, MainWidgets);

private slots:
    void onChooseImage();
    void onDeleteImage();
    void onImageDropped(ImageType imageType, QUrl imageUrl);

    void onRevertChanges();
    void onDownloadFinished(DownloadManagerElement elem);

private:
    Ui::TvShowWidgetSeason* ui;
    TvShow* m_show;
    SeasonNumber m_season;
    QLabel* m_savingWidget;
    QMovie* m_loadingMovie;
    DownloadManager* m_downloadManager;
    void updateImages(QVector<ImageType> images);
};
