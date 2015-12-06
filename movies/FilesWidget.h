#ifndef FILESWIDGET_H
#define FILESWIDGET_H

#include <QEvent>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QWidget>
#include "movies/Movie.h"
#include "data/MovieModel.h"
#include "data/MovieProxyModel.h"
#include "globals/Filter.h"
#include "smallWidgets/AlphabeticalList.h"
#include "smallWidgets/SearchOverlay.h"

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
    void renewModel();
    void selectMovie(Movie *movie);

public slots:
    void restoreLastSelection();
    void setFilter(QList<Filter*> filters, QString text);
    void movieSelectedEmitter();
    void multiScrape();
    void setAlphaListData();

signals:
    void noMovieSelected();
    void movieSelected(Movie*);
    void sigStartSearch();

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void resizeEvent(QResizeEvent *event);

private slots:
    void itemActivated(QModelIndex index, QModelIndex previous);
    void onSortByName();
    void onSortByAdded();
    void onSortByYear();
    void onSortBySeen();
    void onSortByNew();
    void showContextMenu(QPoint point);
    void markAsWatched();
    void markAsUnwatched();
    void loadStreamDetails();
    void markForSync();
    void unmarkForSync();
    void openFolder();
    void scrollToAlpha(QString alpha);
    void onLeftEdge(bool isEdge);
    void onActionMediaStatusColumn();
    void onLabel();
    void onViewUpdated();
    void playMovie(QModelIndex idx);
    void openNfoFile();

private:
    Ui::FilesWidget *ui;
    MovieProxyModel *m_movieProxyModel;
    Movie *m_lastMovie;
    QModelIndex m_lastModelIndex;
    static FilesWidget *m_instance;
    QString m_baseLabelCss;
    QString m_activeLabelCss;
    QMenu *m_contextMenu;
    AlphabeticalList *m_alphaList;
    bool m_mouseIsIn;
};

#endif // FILESWIDGET_H
