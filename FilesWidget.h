#ifndef FILESWIDGET_H
#define FILESWIDGET_H

#include <QLabel>
#include <QWidget>
#include "data/Movie.h"
#include "data/MovieModel.h"
#include "data/MovieProxyModel.h"

namespace Ui {
class FilesWidget;
}

class FilesWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit FilesWidget(QWidget *parent = 0);
    ~FilesWidget();
    void enableRefresh();
    void disableRefresh();
    void showFirstTime();
    void hideFirstTime();
    
signals:
    void noMovieSelected();
    void movieSelected(Movie*);

private slots:
    void startSearch();
    void searchFinished();
    void itemActivated(QModelIndex index);
    void filter(QString filter);
    void tableViewResized(QSize size);

private:
    Ui::FilesWidget *ui;
    MovieProxyModel *m_movieProxyModel;
    QLabel *m_firstTimeLabel;
};

#endif // FILESWIDGET_H
