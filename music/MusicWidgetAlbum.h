#ifndef MUSICWIDGETALBUM_H
#define MUSICWIDGETALBUM_H

#include <QPointer>
#include <QWidget>
#include "../music/Album.h"

namespace Ui {
class MusicWidgetAlbum;
}

class MusicWidgetAlbum : public QWidget
{
    Q_OBJECT

public:
    explicit MusicWidgetAlbum(QWidget *parent = 0);
    ~MusicWidgetAlbum();
    void setAlbum(Album *album);

public slots:
    void onSetEnabled(bool enabled);
    void onClear();
    void onSaveInformation();
    void onStartScraperSearch();

signals:
    void sigSetActionSearchEnabled(bool, MainWidgets);
    void sigSetActionSaveEnabled(bool, MainWidgets);

private:
    Ui::MusicWidgetAlbum *ui;
    QPointer<Album> m_album;

    void updateAlbumInfo();
};

#endif // MUSICWIDGETALBUM_H
