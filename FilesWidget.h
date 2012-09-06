#ifndef FILESWIDGET_H
#define FILESWIDGET_H

#include <QLabel>
#include <QWidget>
#include "data/Movie.h"
#include "data/MovieModel.h"
#include "data/MovieProxyModel.h"
#include "data/MovieDelegate.h"

namespace Ui {
class FilesWidget;
}

/**
 * @brief The FilesWidget class
 * This widget displays a list of movies
 * It's a singleton and gets constructed through the gui,
 * the instance can be retrieved through FilesWidget::instance
 */
class FilesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilesWidget(QWidget *parent = 0);
    ~FilesWidget();
    static FilesWidget *instance();

public slots:
    void restoreLastSelection();
    void setFilter(QString filter);

signals:
    void noMovieSelected();
    void movieSelected(Movie*);

private slots:
    void itemActivated(QModelIndex index, QModelIndex previous);
    void movieSelectedEmitter();

private:
    Ui::FilesWidget *ui;
    MovieProxyModel *m_movieProxyModel;
    MovieDelegate *m_movieDelegate;
    Movie *m_lastMovie;
    QModelIndex m_lastModelIndex;
    static FilesWidget *m_instance;
};

#endif // FILESWIDGET_H
