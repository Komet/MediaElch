#ifndef FILESWIDGET_H
#define FILESWIDGET_H

#include <QLabel>
#include <QMenu>
#include <QWidget>
#include "movies/Movie.h"
#include "data/MovieModel.h"
#include "data/MovieProxyModel.h"
#include "data/MovieDelegate.h"
#include "globals/Filter.h"

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
    QList<Movie*> selectedMovies();

public slots:
    void restoreLastSelection();
    void setFilter(QList<Filter*> filters, QString text);
    void movieSelectedEmitter();

signals:
    void noMovieSelected();
    void movieSelected(Movie*);

private slots:
    void itemActivated(QModelIndex index, QModelIndex previous);
    void onSortByName();
    void onSortByAdded();
    void onSortByYear();
    void onSortBySeen();
    void onSortByNew();
    void showContextMenu(QPoint point);
    void multiScrape();
    void markAsWatched();
    void markAsUnwatched();
    void loadStreamDetails();
    void markForSync();
    void unmarkForSync();
    void openFolder();

private:
    Ui::FilesWidget *ui;
    MovieProxyModel *m_movieProxyModel;
    MovieDelegate *m_movieDelegate;
    Movie *m_lastMovie;
    QModelIndex m_lastModelIndex;
    static FilesWidget *m_instance;
    QString m_baseLabelCss;
    QString m_activeLabelCss;
    QMenu *m_contextMenu;
};

#endif // FILESWIDGET_H
