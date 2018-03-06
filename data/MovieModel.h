#ifndef MOVIEMODEL_H
#define MOVIEMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

#include "movies/Movie.h"

/**
 * @brief The MovieModel class
 */
class MovieModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum MovieRoles
    {
        NameRole = Qt::UserRole + 1,
        FileNameRole
    };
    explicit MovieModel(QObject *parent = nullptr);
    void addMovie(Movie *movie);
    void clear();
    virtual QList<Movie *> movies();
    Movie *movie(int row);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int hasNewMovies();
    static int mediaStatusToColumn(MediaStatusColumns column);
    static QString mediaStatusToText(MediaStatusColumns column);
    static MediaStatusColumns columnToMediaStatus(int column);
    void update();

private slots:
    void onMovieChanged(Movie *movie);

private:
    QList<Movie *> m_movies;
    QIcon m_newIcon;
    QIcon m_syncIcon;
};

#endif // MOVIEMODEL_H
