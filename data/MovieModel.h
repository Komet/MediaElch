#ifndef MOVIEMODEL_H
#define MOVIEMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include "data/Movie.h"

class MovieModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum MovieRoles {
         NameRole = Qt::UserRole + 1,
         FileNameRole
    };
    explicit MovieModel(QObject *parent = 0);
    void addMovie(Movie *movie);
    void clear();
    QList<Movie*> movies();
    Movie *movie(int row);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    
private:
    QList<Movie*> m_movies;

    QIcon m_movieIconRed;
    QIcon m_movieIconGreen;
};

#endif // MOVIEMODEL_H
