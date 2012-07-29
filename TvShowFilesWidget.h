#ifndef TVSHOWFILESWIDGET_H
#define TVSHOWFILESWIDGET_H

#include <QModelIndex>
#include <QWidget>
#include "Globals.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "data/TvShowProxyModel.h"
#include "data/TvShowDelegate.h"

namespace Ui {
class TvShowFilesWidget;
}

class TvShowFilesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowFilesWidget(QWidget *parent = 0);
    ~TvShowFilesWidget();
    void renewModel();
    void setFilter(QString filter);
    static TvShowFilesWidget *instance();

public slots:
    void startSearch();

signals:
    void sigEpisodeSelected(TvShowEpisode *episode);
    void sigTvShowSelected(TvShow *show);
    void sigNothingSelected();

private slots:
    void onItemActivated(QModelIndex index, QModelIndex previous);
    void onItemClicked(QModelIndex index);

private:
    Ui::TvShowFilesWidget *ui;
    TvShowProxyModel *m_tvShowProxyModel;
    TvShowDelegate *m_tvShowDelegate;
    static TvShowFilesWidget *m_instance;
};

#endif // TVSHOWFILESWIDGET_H
