#pragma once

#include "concerts/Concert.h"
#include "export/ExportTemplate.h"
#include "globals/Globals.h"
#include "globals/ScraperResult.h"
#include "movies/Movie.h"
#include "music/Album.h"
#include "music/Artist.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

#include <QObject>
#include <QPointer>
#include <QTableWidgetItem>

class Storage : public QObject
{
    Q_OBJECT
public:
    explicit Storage(QObject* parent, Movie* movie);
    explicit Storage(QObject* parent, Concert* concert);
    explicit Storage(QObject* parent, TvShow* show);
    explicit Storage(QObject* parent, TvShowEpisode* episode);
    explicit Storage(QObject* parent, Artist* artist);
    explicit Storage(QObject* parent, Album* album);
    explicit Storage(QObject* parent, QVector<ScraperSearchResult> results);
    explicit Storage(QObject* parent, QSet<MovieScraperInfos> infosToLoad);
    explicit Storage(QObject* parent, QSet<ShowScraperInfos> infosToLoad);
    explicit Storage(QObject* parent, QSet<ConcertScraperInfos> infosToLoad);
    explicit Storage(QObject* parent, QSet<MusicScraperInfos> infosToLoad);
    explicit Storage(QObject* parent, QVector<ImageType> infosToLoad);
    explicit Storage(QObject* parent, ExportTemplate* exportTemplate);
    explicit Storage(QObject* parent, QHash<MovieScraperInterface*, QString> ids);
    explicit Storage(QObject* parent, QTableWidgetItem* item);
    explicit Storage(QObject* parent, QVector<TvShowEpisode*> episodes);
    Movie* movie() const;
    Concert* concert() const;
    TvShow* show() const;
    TvShowEpisode* episode() const;
    Artist* artist() const;
    Album* album() const;
    QVector<ScraperSearchResult> results() const;
    QSet<MovieScraperInfos> movieInfosToLoad() const;
    QSet<ShowScraperInfos> showInfosToLoad() const;
    QSet<ConcertScraperInfos> concertInfosToLoad() const;
    QSet<MusicScraperInfos> musicInfosToLoad() const;
    QVector<ImageType> imageInfosToLoad() const;
    ExportTemplate* exportTemplate() const;
    QHash<MovieScraperInterface*, QString> ids() const;
    QTableWidgetItem* tableWidgetItem() const;
    QVector<TvShowEpisode*> episodes() const;
    static QVariant toVariant(QObject* parent, Movie* movie);
    static QVariant toVariant(QObject* parent, Concert* concert);
    static QVariant toVariant(QObject* parent, TvShow* show);
    static QVariant toVariant(QObject* parent, TvShowEpisode* episode);
    static QVariant toVariant(QObject* parent, Artist* artist);
    static QVariant toVariant(QObject* parent, Album* album);
    static QVariant toVariant(QObject* parent, QVector<ScraperSearchResult> results);
    static QVariant toVariant(QObject* parent, QSet<MovieScraperInfos> infosToLoad);
    static QVariant toVariant(QObject* parent, QSet<ShowScraperInfos> infosToLoad);
    static QVariant toVariant(QObject* parent, QSet<ConcertScraperInfos> infosToLoad);
    static QVariant toVariant(QObject* parent, QSet<MusicScraperInfos> infosToLoad);
    static QVariant toVariant(QObject* parent, QVector<ImageType> infosToLoad);
    static QVariant toVariant(QObject* parent, ExportTemplate* exportTemplate);
    static QVariant toVariant(QObject* parent, QHash<MovieScraperInterface*, QString> ids);
    static QVariant toVariant(QObject* parent, QTableWidgetItem* item);
    static QVariant toVariant(QObject* parent, QVector<TvShowEpisode*> episodes);

private:
    QPointer<Movie> m_movie;
    QPointer<Concert> m_concert;
    QPointer<TvShow> m_show;
    QPointer<TvShowEpisode> m_episode;
    QPointer<Artist> m_artist;
    QPointer<Album> m_album;
    QVector<ScraperSearchResult> m_results;
    QSet<MusicScraperInfos> m_musicInfosToLoad;
    QSet<MovieScraperInfos> m_movieInfosToLoad;
    QSet<ShowScraperInfos> m_showInfosToLoad;
    QSet<ConcertScraperInfos> m_concertInfosToLoad;
    QVector<ImageType> m_imageInfosToLoad;
    QPointer<ExportTemplate> m_exportTemplate;
    QHash<MovieScraperInterface*, QString> m_ids;
    QTableWidgetItem* m_tableWidgetItem = nullptr;
    QVector<TvShowEpisode*> m_episodes;
};

Q_DECLARE_METATYPE(Storage*)
