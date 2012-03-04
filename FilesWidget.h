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

public slots:
    void restoreLastSelection();

signals:
    void noMovieSelected();
    void movieSelected(Movie*);

private slots:
    void startSearch();
    void searchFinished();
    void itemActivated(QModelIndex index, QModelIndex previous);
    void filter(QString filter);
    void tableViewResized(QSize size);
    void movieSelectedEmitter();

private:
    Ui::FilesWidget *ui;
    MovieProxyModel *m_movieProxyModel;
    Movie *m_lastMovie;
    QLabel *m_firstTimeLabel;
    QModelIndex m_lastModelIndex;
    bool m_emitMovieSelected;
};

#endif // FILESWIDGET_H
