#ifndef TVSHOWFILESWIDGET_H
#define TVSHOWFILESWIDGET_H

#include <QMenu>
#include <QModelIndex>
#include <QWidget>
#include "globals/Globals.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "data/TvShowProxyModel.h"
#include "data/TvShowDelegate.h"
#include "globals/Filter.h"

namespace Ui {
class TvShowFilesWidget;
}

/**
 * @brief The TvShowFilesWidget class
 */
class TvShowFilesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowFilesWidget(QWidget *parent = 0);
    ~TvShowFilesWidget();
    void setFilter(QList<Filter*> filters, QString text);
    static TvShowFilesWidget *instance();

public slots:
    void renewModel();

signals:
    void sigEpisodeSelected(TvShowEpisode *episode);
    void sigTvShowSelected(TvShow *show);
    void sigNothingSelected();

private slots:
    void onItemActivated(QModelIndex index, QModelIndex previous);
    void onItemClicked(QModelIndex index);
    void showContextMenu(QPoint point);
    void scanForEpisodes();

private:
    Ui::TvShowFilesWidget *ui;
    TvShowProxyModel *m_tvShowProxyModel;
    TvShowDelegate *m_tvShowDelegate;
    static TvShowFilesWidget *m_instance;
    QMenu *m_contextMenu;
};

#endif // TVSHOWFILESWIDGET_H
