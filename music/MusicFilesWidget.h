#ifndef MUSICFILESWIDGET_H
#define MUSICFILESWIDGET_H

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

public slots:
    void setFilter(QList<Filter*> filters, QString text);

signals:
    void sigArtistSelected(Artist*);
    void sigAlbumSelected(Album*);
    void sigNothingSelected();

private slots:
    void onItemSelected(QModelIndex index);

private:
    Ui::MusicFilesWidget *ui;
    static MusicFilesWidget *m_instance;
    MusicProxyModel *m_proxyModel;
};

#endif // MUSICFILESWIDGET_H
