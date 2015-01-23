#ifndef MUSICWIDGET_H
#define MUSICWIDGET_H

#include <QWidget>
#include "Artist.h"
#include "Album.h"

namespace Ui {
class MusicWidget;
}

class MusicWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MusicWidget(QWidget *parent = 0);
    ~MusicWidget();

public slots:
    void onArtistSelected(Artist *artist);
    void onAlbumSelected(Album *album);
    void onSetEnabledTrue(Artist *artist);
    void onSetEnabledTrue(Album *album);
    void onClear();
    void onSetDisabledTrue();
    void setBigWindow(bool bigWindow);
    void onStartScraperSearch();
    void onSaveInformation();
    void onSaveAll();

signals:
    void sigSetActionSearchEnabled(bool, MainWidgets);
    void sigSetActionSaveEnabled(bool, MainWidgets);
    void sigDownloadsStarted(QString,int);
    void sigDownloadsProgress(int,int,int);
    void sigDownloadsFinished(int);

private:
    Ui::MusicWidget *ui;
};

#endif // MUSICWIDGET_H
