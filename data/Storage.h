#ifndef STORAGE_H
#define STORAGE_H

#include <QObject>
#include <QPointer>
#include "data/Concert.h"
#include "globals/Globals.h"
#include "movies/Movie.h"

class Storage : public QObject
{
    Q_OBJECT
public:
    explicit Storage(QObject *parent, Movie *movie);
    explicit Storage(QObject *parent, Concert *concert);
    explicit Storage(QObject *parent, QList<ScraperSearchResult> results);
    Movie *movie();
    Concert *concert();
    QList<ScraperSearchResult> results();
    static QVariant toVariant(QObject *parent, Movie *movie);
    static QVariant toVariant(QObject *parent, Concert *concert);
    static QVariant toVariant(QObject *parent, QList<ScraperSearchResult> results);

private:
    QPointer<Movie> m_movie;
    QPointer<Concert> m_concert;
    QList<ScraperSearchResult> m_results;
};

Q_DECLARE_METATYPE(Storage*)

#endif // STORAGE_H
