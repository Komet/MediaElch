#pragma once

#include "movies/Movie.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <QModelIndex>
#include <QVector>

class MovieModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Roles
    {
        NameRole = Qt::UserRole,
        InfoLoadedRole = Qt::UserRole + 1,
        HasChangedRole = Qt::UserRole + 2,
        ReleasedRole = Qt::UserRole + 3,
        HasWatchedRole = Qt::UserRole + 4,
        FileLastModifiedRole = Qt::UserRole + 5,
        SyncNeededRole = Qt::UserRole + 6,
        FileNameRole = Qt::UserRole + 7,
        SortTitleRole = Qt::UserRole + 8
    };

    explicit MovieModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;

    virtual QVector<Movie*> movies();
    Movie* movie(int row);
    void addMovie(Movie* movie);
    void addMovies(const QVector<Movie*>& movies);
    void update();
    void clear();
    int countNewMovies();

    static int mediaStatusToColumn(MediaStatusColumn column);
    static QString mediaStatusToText(MediaStatusColumn column);
    static MediaStatusColumn columnToMediaStatus(int column);

private slots:
    void onMovieChanged(Movie* movie);

private:
    QVector<Movie*> m_movies;
    QIcon m_newIcon;
    QIcon m_syncIcon;
};
