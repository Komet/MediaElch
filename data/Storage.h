#ifndef STORAGE_H
#define STORAGE_H

#include <QObject>
#include <QPointer>
#include "globals/Globals.h"
#include "movies/Movie.h"

class Storage : public QObject
{
    Q_OBJECT
public:
    explicit Storage(QObject *parent, Movie *movie);
    explicit Storage(QObject *parent, QList<ScraperSearchResult> results);
    Movie *movie();
    QList<ScraperSearchResult> results();
    static QVariant toVariant(QObject *parent, Movie *movie);
    static QVariant toVariant(QObject *parent, QList<ScraperSearchResult> results);

private:
    QPointer<Movie> m_movie;
    QList<ScraperSearchResult> m_results;
};

Q_DECLARE_METATYPE(Storage*)

#endif // STORAGE_H
