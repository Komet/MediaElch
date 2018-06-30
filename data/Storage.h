#ifndef STORAGE_H
#define STORAGE_H

#include <QObject>
#include <QPointer>
#include <QTableWidgetItem>

#include "data/Concert.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "export/ExportTemplate.h"
#include "globals/Globals.h"
#include "movies/Movie.h"
#include "music/Album.h"
#include "music/Artist.h"

class Storage : public QObject
{
    Q_OBJECT
public:
    explicit Storage(QObject *parent, Movie *movie);
    explicit Storage(QObject *parent, Concert *concert);
    explicit Storage(QObject *parent, TvShow *show);
    explicit Storage(QObject *parent, TvShowEpisode *episode);
    explicit Storage(QObject *parent, Artist *artist);
    explicit Storage(QObject *parent, Album *album);
    explicit Storage(QObject *parent, QList<ScraperSearchResult> results);
    explicit Storage(QObject *parent, QList<int> infosToLoad);
    explicit Storage(QObject *parent, QList<MovieScraperInfos> infosToLoad);
    explicit Storage(QObject *parent, QList<TvShowScraperInfos> infosToLoad);
    explicit Storage(QObject *parent, QList<ConcertScraperInfos> infosToLoad);
    explicit Storage(QObject *parent, QList<ImageType> infosToLoad);
    explicit Storage(QObject *parent, ExportTemplate *exportTemplate);
    explicit Storage(QObject *parent, QMap<ScraperInterface *, QString> ids);
    explicit Storage(QObject *parent, QTableWidgetItem *item);
    explicit Storage(QObject *parent, QList<TvShowEpisode *> episodes);
    Movie *movie() const;
    Concert *concert() const;
    TvShow *show() const;
    TvShowEpisode *episode() const;
    Artist *artist() const;
    Album *album() const;
    QList<ScraperSearchResult> results() const;
    QList<int> infosToLoad() const;
    QList<MovieScraperInfos> movieInfosToLoad() const;
    QList<TvShowScraperInfos> showInfosToLoad() const;
    QList<ConcertScraperInfos> concertInfosToLoad() const;
    QList<ImageType> imageInfosToLoad() const;
    ExportTemplate *exportTemplate() const;
    QMap<ScraperInterface *, QString> ids() const;
    QTableWidgetItem *tableWidgetItem() const;
    QList<TvShowEpisode *> episodes() const;
    static QVariant toVariant(QObject *parent, Movie *movie);
    static QVariant toVariant(QObject *parent, Concert *concert);
    static QVariant toVariant(QObject *parent, TvShow *show);
    static QVariant toVariant(QObject *parent, TvShowEpisode *episode);
    static QVariant toVariant(QObject *parent, Artist *artist);
    static QVariant toVariant(QObject *parent, Album *album);
    static QVariant toVariant(QObject *parent, QList<ScraperSearchResult> results);
    static QVariant toVariant(QObject *parent, QList<int> infosToLoad);
    static QVariant toVariant(QObject *parent, QList<MovieScraperInfos> infosToLoad);
    static QVariant toVariant(QObject *parent, QList<TvShowScraperInfos> infosToLoad);
    static QVariant toVariant(QObject *parent, QList<ConcertScraperInfos> infosToLoad);
    static QVariant toVariant(QObject *parent, QList<ImageType> infosToLoad);
    static QVariant toVariant(QObject *parent, ExportTemplate *exportTemplate);
    static QVariant toVariant(QObject *parent, QMap<ScraperInterface *, QString> ids);
    static QVariant toVariant(QObject *parent, QTableWidgetItem *item);
    static QVariant toVariant(QObject *parent, QList<TvShowEpisode *> episodes);

private:
    QPointer<Movie> m_movie;
    QPointer<Concert> m_concert;
    QPointer<TvShow> m_show;
    QPointer<TvShowEpisode> m_episode;
    QPointer<Artist> m_artist;
    QPointer<Album> m_album;
    QList<ScraperSearchResult> m_results;
    QList<int> m_infosToLoad;
    QList<MovieScraperInfos> m_movieInfosToLoad;
    QList<TvShowScraperInfos> m_showInfosToLoad;
    QList<ConcertScraperInfos> m_concertInfosToLoad;
    QList<ImageType> m_imageInfosToLoad;
    QPointer<ExportTemplate> m_exportTemplate;
    QMap<ScraperInterface *, QString> m_ids;
    QTableWidgetItem *m_tableWidgetItem = nullptr;
    QList<TvShowEpisode *> m_episodes;
};

Q_DECLARE_METATYPE(Storage *)

#endif // STORAGE_H
