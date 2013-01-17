#ifndef STORAGE_H
#define STORAGE_H

#include <QObject>
#include <QPointer>
#include "data/Concert.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "globals/Globals.h"
#include "movies/Movie.h"

class Storage : public QObject
{
    Q_OBJECT
public:
    explicit Storage(QObject *parent, Movie *movie);
    explicit Storage(QObject *parent, Concert *concert);
    explicit Storage(QObject *parent, TvShow *show);
    explicit Storage(QObject *parent, TvShowEpisode *episode);
    explicit Storage(QObject *parent, QList<ScraperSearchResult> results);
    Movie *movie();
    Concert *concert();
    TvShow *show();
    TvShowEpisode *episode();
    QList<ScraperSearchResult> results();
    static QVariant toVariant(QObject *parent, Movie *movie);
    static QVariant toVariant(QObject *parent, Concert *concert);
    static QVariant toVariant(QObject *parent, TvShow *show);
    static QVariant toVariant(QObject *parent, TvShowEpisode *episode);
    static QVariant toVariant(QObject *parent, QList<ScraperSearchResult> results);

private:
    QPointer<Movie> m_movie;
    QPointer<Concert> m_concert;
    QPointer<TvShow> m_show;
    QPointer<TvShowEpisode> m_episode;
    QList<ScraperSearchResult> m_results;
};

Q_DECLARE_METATYPE(Storage*)

#endif // STORAGE_H
