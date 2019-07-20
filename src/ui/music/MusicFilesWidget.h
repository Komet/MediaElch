#pragma once

#include "globals/Filter.h"
#include "music/Album.h"
#include "music/Artist.h"
#include "music/MusicProxyModel.h"

#include <QMenu>
#include <QModelIndex>
#include <QWidget>

namespace Ui {
class MusicFilesWidget;
}

class MusicFilesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MusicFilesWidget(QWidget* parent = nullptr);
    ~MusicFilesWidget() override;
    static MusicFilesWidget* instance();
    QVector<Artist*> selectedArtists();
    QVector<Album*> selectedAlbums();

public slots:
    void setFilter(QVector<Filter*> filters, QString text);
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
    Ui::MusicFilesWidget* ui;
    static MusicFilesWidget* m_instance;
    MusicProxyModel* m_proxyModel;
    QMenu* m_contextMenu = nullptr;
};
