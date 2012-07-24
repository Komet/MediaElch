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
    static FilesWidget *instance();

public slots:
    void restoreLastSelection();
    void setFilter(QString filter);

signals:
    void noMovieSelected();
    void movieSelected(Movie*);
    void setRefreshButtonEnabled(bool, MainWidgets);

private slots:
    void startSearch();
    void searchFinished();
    void itemActivated(QModelIndex index, QModelIndex previous);
    void movieSelectedEmitter();

private:
    Ui::FilesWidget *ui;
    MovieProxyModel *m_movieProxyModel;
    Movie *m_lastMovie;
    QModelIndex m_lastModelIndex;
    static FilesWidget *m_instance;
};

#endif // FILESWIDGET_H
