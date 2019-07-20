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
    enum MovieRoles
    {
        NameRole = Qt::UserRole + 1,
        FileNameRole
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
