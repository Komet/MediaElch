#ifndef TVSHOWFILESWIDGET_H
#define TVSHOWFILESWIDGET_H

#include <QModelIndex>
#include <QWidget>
#include "Globals.h"
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
    void enableRefresh();
    void disableRefresh();

public slots:
    void startSearch();

signals:
    void setRefreshButtonEnabled(bool, MainWidgets);

private slots:
    void onItemClicked(QModelIndex index);
    void searchFinished();

private:
    Ui::TvShowFilesWidget *ui;
    TvShowProxyModel *m_tvShowProxyModel;
    TvShowDelegate *m_tvShowDelegate;
};

#endif // TVSHOWFILESWIDGET_H
