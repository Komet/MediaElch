#ifndef MUSICFILESWIDGET_H
#define MUSICFILESWIDGET_H

#include <QMenu>
#include <QModelIndex>
#include <QWidget>

#include "globals/Filter.h"
#include "Artist.h"
#include "Album.h"
#include "MusicProxyModel.h"

namespace Ui {
class MusicFilesWidget;
}

class MusicFilesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MusicFilesWidget(QWidget *parent = 0);
    ~MusicFilesWidget();
    static MusicFilesWidget *instance();
    QList<Artist*> selectedArtists();
    QList<Album*> selectedAlbums();

public slots:
    void setFilter(QList<Filter*> filters, QString text);
    void multiScrape();

signals:
    void sigArtistSelected(Artist*);
    void sigAlbumSelected(Album*);
    void sigNothingSelected();

private slots:
    void onItemSelected(QModelIndex index);
    void updateStatusLabel();
    void onOpenFolder();
    void onOpenNfo();
    void showContextMenu(QPoint point);

private:
    Ui::MusicFilesWidget *ui;
    static MusicFilesWidget *m_instance;
    MusicProxyModel *m_proxyModel;
    QMenu *m_contextMenu;
};

#endif // MUSICFILESWIDGET_H
