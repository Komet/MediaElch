#include "Storage.h"

Storage::Storage(QObject* parent, Movie* movie) : QObject(parent), m_movie{QPointer<Movie>(movie)}
{
}

Storage::Storage(QObject* parent, Concert* concert) : QObject(parent), m_concert{QPointer<Concert>(concert)}
{
}

Storage::Storage(QObject* parent, TvShow* show) : QObject(parent), m_show{QPointer<TvShow>(show)}
{
}

Storage::Storage(QObject* parent, TvShowEpisode* episode) : QObject(parent), m_episode{QPointer<TvShowEpisode>(episode)}
{
}

Storage::Storage(QObject* parent, Artist* artist) : QObject(parent), m_artist{QPointer<Artist>(artist)}
{
}

Storage::Storage(QObject* parent, Album* album) : QObject(parent), m_album{QPointer<Album>(album)}
{
}

Storage::Storage(QObject* parent, QVector<ScraperSearchResult> results) : QObject(parent), m_results{results}
{
}

Storage::Storage(QObject* parent, QVector<MovieScraperInfos> infosToLoad) :
    QObject(parent),
    m_movieInfosToLoad{infosToLoad}
{
}

Storage::Storage(QObject* parent, QVector<TvShowScraperInfos> infosToLoad) :
    QObject(parent),
    m_showInfosToLoad{infosToLoad}
{
}

Storage::Storage(QObject* parent, QVector<ConcertScraperInfos> infosToLoad) :
    QObject(parent),
    m_concertInfosToLoad{infosToLoad}
{
}

Storage::Storage(QObject* parent, QVector<MusicScraperInfos> infosToLoad) :
    QObject(parent),
    m_musicInfosToLoad{infosToLoad}
{
}


Storage::Storage(QObject* parent, QVector<ImageType> infosToLoad) : QObject(parent), m_imageInfosToLoad{infosToLoad}
{
}

Storage::Storage(QObject* parent, ExportTemplate* exportTemplate) : QObject(parent), m_exportTemplate{exportTemplate}
{
}

Storage::Storage(QObject* parent, QMap<MovieScraperInterface*, QString> ids) : QObject(parent), m_ids{ids}
{
}

Storage::Storage(QObject* parent, QTableWidgetItem* item) : QObject(parent), m_tableWidgetItem{item}
{
}

Storage::Storage(QObject* parent, QVector<TvShowEpisode*> episodes) : QObject(parent), m_episodes{episodes}
{
}

Movie* Storage::movie() const
{
    if (m_movie != nullptr) {
        return m_movie;
    }
    return nullptr;
}

Concert* Storage::concert() const
{
    if (m_concert != nullptr) {
        return m_concert;
    }
    return nullptr;
}

TvShow* Storage::show() const
{
    if (m_show != nullptr) {
        return m_show;
    }
    return nullptr;
}

TvShowEpisode* Storage::episode() const
{
    if (m_episode != nullptr) {
        return m_episode;
    }
    return nullptr;
}

Artist* Storage::artist() const
{
    if (m_artist != nullptr) {
        return m_artist;
    }
    return nullptr;
}

Album* Storage::album() const
{
    if (m_album != nullptr) {
        return m_album;
    }
    return nullptr;
}

ExportTemplate* Storage::exportTemplate() const
{
    if (m_exportTemplate != nullptr) {
        return m_exportTemplate;
    }
    return nullptr;
}

QTableWidgetItem* Storage::tableWidgetItem() const
{
    if (m_tableWidgetItem != nullptr) {
        return m_tableWidgetItem;
    }
    return nullptr;
}

QMap<MovieScraperInterface*, QString> Storage::ids() const
{
    return m_ids;
}

QVector<TvShowEpisode*> Storage::episodes() const
{
    return m_episodes;
}

QVariant Storage::toVariant(QObject* parent, Movie* movie)
{
    auto storage = new Storage(parent, movie);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, Concert* concert)
{
    const auto storage = new Storage(parent, concert);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, TvShow* show)
{
    const auto storage = new Storage(parent, show);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, TvShowEpisode* episode)
{
    const auto storage = new Storage(parent, episode);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, Artist* artist)
{
    const auto storage = new Storage(parent, artist);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, Album* album)
{
    const auto storage = new Storage(parent, album);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QVector<ScraperSearchResult> results)
{
    Storage* const storage = new Storage(parent, results);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QVector<MovieScraperInfos> infosToLoad)
{
    Storage* const storage = new Storage(parent, infosToLoad);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QVector<TvShowScraperInfos> infosToLoad)
{
    Storage* const storage = new Storage(parent, infosToLoad);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QVector<ConcertScraperInfos> infosToLoad)
{
    Storage* const storage = new Storage(parent, infosToLoad);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QVector<MusicScraperInfos> infosToLoad)
{
    Storage* const storage = new Storage(parent, infosToLoad);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QVector<ImageType> infosToLoad)
{
    Storage* const storage = new Storage(parent, infosToLoad);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, ExportTemplate* exportTemplate)
{
    const auto storage = new Storage(parent, exportTemplate);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QMap<MovieScraperInterface*, QString> ids)
{
    Storage* const storage = new Storage(parent, ids);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QTableWidgetItem* item)
{
    const auto storage = new Storage(parent, item);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QVector<TvShowEpisode*> episodes)
{
    Storage* const storage = new Storage(parent, episodes);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVector<ScraperSearchResult> Storage::results() const
{
    return m_results;
}

QVector<MovieScraperInfos> Storage::movieInfosToLoad() const
{
    return m_movieInfosToLoad;
}

QVector<TvShowScraperInfos> Storage::showInfosToLoad() const
{
    return m_showInfosToLoad;
}

QVector<ConcertScraperInfos> Storage::concertInfosToLoad() const
{
    return m_concertInfosToLoad;
}

QVector<MusicScraperInfos> Storage::musicInfosToLoad() const
{
    return m_musicInfosToLoad;
}

QVector<ImageType> Storage::imageInfosToLoad() const
{
    return m_imageInfosToLoad;
}
