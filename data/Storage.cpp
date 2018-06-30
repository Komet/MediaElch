#include "Storage.h"

Storage::Storage(QObject *parent, Movie *movie) : QObject(parent)
{
    m_movie = QPointer<Movie>(movie);
}

Storage::Storage(QObject *parent, Concert *concert) : QObject(parent), m_concert{QPointer<Concert>(concert)}
{
}

Storage::Storage(QObject *parent, TvShow *show) : QObject(parent), m_show{QPointer<TvShow>(show)}
{
}

Storage::Storage(QObject *parent, TvShowEpisode *episode) : QObject(parent), m_episode{QPointer<TvShowEpisode>(episode)}
{
}

Storage::Storage(QObject *parent, Artist *artist) : QObject(parent), m_artist{QPointer<Artist>(artist)}
{
}

Storage::Storage(QObject *parent, Album *album) : QObject(parent), m_album{QPointer<Album>(album)}
{
}

Storage::Storage(QObject *parent, QList<ScraperSearchResult> results) : QObject(parent), m_results{results}
{
}

Storage::Storage(QObject *parent, QList<MovieScraperInfos> infosToLoad) :
    QObject(parent),
    m_movieInfosToLoad{infosToLoad}
{
}

Storage::Storage(QObject *parent, QList<TvShowScraperInfos> infosToLoad) :
    QObject(parent),
    m_showInfosToLoad{infosToLoad}
{
}

Storage::Storage(QObject *parent, QList<ConcertScraperInfos> infosToLoad) :
    QObject(parent),
    m_concertInfosToLoad{infosToLoad}
{
}

Storage::Storage(QObject *parent, QList<MusicScraperInfos> infosToLoad) :
    QObject(parent),
    m_musicInfosToLoad{infosToLoad}
{
}


Storage::Storage(QObject *parent, QList<ImageType> infosToLoad) : QObject(parent), m_imageInfosToLoad{infosToLoad}
{
}

Storage::Storage(QObject *parent, ExportTemplate *exportTemplate) : QObject(parent), m_exportTemplate{exportTemplate}
{
}

Storage::Storage(QObject *parent, QMap<ScraperInterface *, QString> ids) : QObject(parent), m_ids{ids}
{
}

Storage::Storage(QObject *parent, QTableWidgetItem *item) : QObject(parent), m_tableWidgetItem{item}
{
}

Storage::Storage(QObject *parent, QList<TvShowEpisode *> episodes) : QObject(parent), m_episodes{episodes}
{
}

Movie *Storage::movie() const
{
    if (m_movie) {
        return m_movie;
    }
    return nullptr;
}

Concert *Storage::concert() const
{
    if (m_concert) {
        return m_concert;
    }
    return nullptr;
}

TvShow *Storage::show() const
{
    if (m_show) {
        return m_show;
    }
    return nullptr;
}

TvShowEpisode *Storage::episode() const
{
    if (m_episode) {
        return m_episode;
    }
    return nullptr;
}

Artist *Storage::artist() const
{
    if (m_artist) {
        return m_artist;
    }
    return nullptr;
}

Album *Storage::album() const
{
    if (m_album) {
        return m_album;
    }
    return nullptr;
}

ExportTemplate *Storage::exportTemplate() const
{
    if (m_exportTemplate) {
        return m_exportTemplate;
    }
    return nullptr;
}

QTableWidgetItem *Storage::tableWidgetItem() const
{
    if (m_tableWidgetItem) {
        return m_tableWidgetItem;
    }
    return nullptr;
}

QMap<ScraperInterface *, QString> Storage::ids() const
{
    return m_ids;
}

QList<TvShowEpisode *> Storage::episodes() const
{
    return m_episodes;
}

QVariant Storage::toVariant(QObject *parent, Movie *movie)
{
    auto storage = new Storage(parent, movie);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, Concert *concert)
{
    const auto storage = new Storage(parent, concert);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, TvShow *show)
{
    const auto storage = new Storage(parent, show);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, TvShowEpisode *episode)
{
    const auto storage = new Storage(parent, episode);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, Artist *artist)
{
    const auto storage = new Storage(parent, artist);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, Album *album)
{
    const auto storage = new Storage(parent, album);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QList<ScraperSearchResult> results)
{
    Storage *const storage = new Storage(parent, results);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QList<MovieScraperInfos> infosToLoad)
{
    Storage *const storage = new Storage(parent, infosToLoad);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QList<TvShowScraperInfos> infosToLoad)
{
    Storage *const storage = new Storage(parent, infosToLoad);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QList<ConcertScraperInfos> infosToLoad)
{
    Storage *const storage = new Storage(parent, infosToLoad);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QList<MusicScraperInfos> infosToLoad)
{
    Storage *const storage = new Storage(parent, infosToLoad);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QList<ImageType> infosToLoad)
{
    Storage *const storage = new Storage(parent, infosToLoad);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, ExportTemplate *exportTemplate)
{
    const auto storage = new Storage(parent, exportTemplate);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QMap<ScraperInterface *, QString> ids)
{
    Storage *const storage = new Storage(parent, ids);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QTableWidgetItem *item)
{
    const auto storage = new Storage(parent, item);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QList<TvShowEpisode *> episodes)
{
    Storage *const storage = new Storage(parent, episodes);
    QVariant var;
    var.setValue(storage);
    return var;
}

QList<ScraperSearchResult> Storage::results() const
{
    return m_results;
}

QList<MovieScraperInfos> Storage::movieInfosToLoad() const
{
    return m_movieInfosToLoad;
}

QList<TvShowScraperInfos> Storage::showInfosToLoad() const
{
    return m_showInfosToLoad;
}

QList<ConcertScraperInfos> Storage::concertInfosToLoad() const
{
    return m_concertInfosToLoad;
}

QList<MusicScraperInfos> Storage::musicInfosToLoad() const
{
    return m_musicInfosToLoad;
}

QList<ImageType> Storage::imageInfosToLoad() const
{
    return m_imageInfosToLoad;
}
