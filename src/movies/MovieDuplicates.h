#ifndef MOVIEDUPLICATES_H
#define MOVIEDUPLICATES_H

#include <QList>
#include <QMap>
#include <QModelIndex>
#include <QWidget>

namespace Ui {
class MovieDuplicates;
}

class Movie;
class MovieProxyModel;
class QMenu;

class MovieDuplicates : public QWidget
{
    Q_OBJECT

public:
    explicit MovieDuplicates(QWidget *parent = nullptr);
    ~MovieDuplicates() override;

signals:
    void sigJumpToMovie(Movie *);

private slots:
    void detectDuplicates();
    void onItemActivated(QModelIndex /*index*/, QModelIndex /*previous*/);

    void showContextMenu(QPoint point);
    void onOpenDetailPage();
    void onOpenFolder();
    void onOpenNfo();
    void onJumpToMovie(const QModelIndex &index);

private:
    void createContextMenu();
    Movie *activeMovie();

    Ui::MovieDuplicates *ui;
    MovieProxyModel *m_movieProxyModel;
    QMenu *m_contextMenu;

    QMap<Movie *, QList<Movie *>> m_duplicateMovies;
};

#endif // MOVIEDUPLICATES_H
