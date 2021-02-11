#pragma once

#include "globals/Filter.h"

#include <QEvent>
#include <QLabel>
#include <QMenu>
#include <QModelIndex>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QWidget>

namespace Ui {
class MovieFilesWidget;
}

class AlphabeticalList;
class Movie;
class MovieProxyModel;

/// \brief   This widget displays a list of movies
/// \details It's a singleton and gets constructed through the gui,
///          the instance can be retrieved through MovieFilesWidget::instance
class MovieFilesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovieFilesWidget(QWidget* parent = nullptr);
    ~MovieFilesWidget() override;
    static MovieFilesWidget* instance();
    QVector<Movie*> selectedMovies();
    void renewModel();
    void selectMovie(Movie* movie);
    void selectIndex(const QModelIndex& index);

public slots:
    void restoreLastSelection();
    void setFilter(QVector<Filter*> filters, QString text);
    void movieSelectedEmitter();
    void multiScrape();
    void setAlphaListData();

signals:
    void noMovieSelected();
    void movieSelected(Movie*);
    void sigStartSearch();

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

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
    void updateStatusLabel();
    void playMovie(QModelIndex idx);
    void openNfoFile();

private:
    Ui::MovieFilesWidget* ui;
    MovieProxyModel* m_movieProxyModel;
    Movie* m_lastMovie;
    QModelIndex m_lastModelIndex;
    static MovieFilesWidget* m_instance;
    QMenu* m_contextMenu = nullptr;
    AlphabeticalList* m_alphaList;
    bool m_mouseIsIn;

    void updateSort(SortBy sortBy);
};
