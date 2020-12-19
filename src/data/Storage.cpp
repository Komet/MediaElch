#include "Storage.h"

#include <utility>

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

Storage::Storage(QObject* parent, QVector<ScraperSearchResult> results) : QObject(parent), m_results{std::move(results)}
{
}

Storage::Storage(QObject* parent, QSet<MovieScraperInfo> infosToLoad) :
    QObject(parent), m_movieInfosToLoad{std::move(infosToLoad)}
{
}

Storage::Storage(QObject* parent, QSet<ShowScraperInfo> infosToLoad) :
    QObject(parent), m_showDetailsToLoad{std::move(infosToLoad)}
{
}

Storage::Storage(QObject* parent, QSet<ConcertScraperInfo> infosToLoad) :
    QObject(parent), m_concertInfosToLoad{std::move(infosToLoad)}
{
}

Storage::Storage(QObject* parent, QSet<MusicScraperInfo> infosToLoad) :
    QObject(parent), m_musicInfosToLoad{std::move(infosToLoad)}
{
}


Storage::Storage(QObject* parent, QVector<ImageType> infosToLoad) :
    QObject(parent), m_imageInfosToLoad{std::move(infosToLoad)}
{
}

Storage::Storage(QObject* parent, ExportTemplate* exportTemplate) : QObject(parent), m_exportTemplate{exportTemplate}
{
}

Storage::Storage(QObject* parent, QHash<mediaelch::scraper::MovieScraper*, QString> ids) :
    QObject(parent), m_ids{std::move(ids)}
{
}

Storage::Storage(QObject* parent, QTableWidgetItem* item) : QObject(parent), m_tableWidgetItem{item}
{
}

Storage::Storage(QObject* parent, QVector<TvShowEpisode*> episodes) : QObject(parent), m_episodes{std::move(episodes)}
{
}

Movie* Storage::movie() const
{
    return m_movie;
}

Concert* Storage::concert() const
{
    return m_concert;
}

TvShow* Storage::show() const
{
    return m_show;
}

TvShowEpisode* Storage::episode() const
{
    return m_episode;
}

Artist* Storage::artist() const
{
    return m_artist;
}

Album* Storage::album() const
{
    return m_album;
}

ExportTemplate* Storage::exportTemplate() const
{
    return m_exportTemplate;
}

QTableWidgetItem* Storage::tableWidgetItem() const
{
    return m_tableWidgetItem;
}

QHash<mediaelch::scraper::MovieScraper*, QString> Storage::ids() const
{
    return m_ids;
}

QVector<TvShowEpisode*> Storage::episodes() const
{
    return m_episodes;
}

QVariant Storage::toVariant(QObject* parent, Movie* movie)
{
    auto* storage = new Storage(parent, movie);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, Concert* concert)
{
    auto* const storage = new Storage(parent, concert);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, TvShow* show)
{
    auto* const storage = new Storage(parent, show);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, TvShowEpisode* episode)
{
    auto* const storage = new Storage(parent, episode);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, Artist* artist)
{
    auto* const storage = new Storage(parent, artist);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, Album* album)
{
    auto* const storage = new Storage(parent, album);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QVector<ScraperSearchResult> results)
{
    auto* const storage = new Storage(parent, std::move(results));
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QSet<MovieScraperInfo> infosToLoad)
{
    auto* const storage = new Storage(parent, std::move(infosToLoad));
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QSet<ShowScraperInfo> infosToLoad)
{
    auto* const storage = new Storage(parent, std::move(infosToLoad));
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QSet<ConcertScraperInfo> infosToLoad)
{
    auto* const storage = new Storage(parent, std::move(infosToLoad));
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QSet<MusicScraperInfo> infosToLoad)
{
    auto* const storage = new Storage(parent, std::move(infosToLoad));
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QVector<ImageType> infosToLoad)
{
    auto* const storage = new Storage(parent, std::move(infosToLoad));
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, ExportTemplate* exportTemplate)
{
    auto* const storage = new Storage(parent, exportTemplate);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QHash<mediaelch::scraper::MovieScraper*, QString> ids)
{
    auto* const storage = new Storage(parent, std::move(ids));
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QTableWidgetItem* item)
{
    auto* const storage = new Storage(parent, item);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject* parent, QVector<TvShowEpisode*> episodes)
{
    auto* const storage = new Storage(parent, std::move(episodes));
    QVariant var;
    var.setValue(storage);
    return var;
}

QVector<ScraperSearchResult> Storage::results() const
{
    return m_results;
}

QSet<MovieScraperInfo> Storage::movieInfosToLoad() const
{
    return m_movieInfosToLoad;
}

QSet<ShowScraperInfo> Storage::showInfosToLoad() const
{
    return m_showDetailsToLoad;
}

QSet<ConcertScraperInfo> Storage::concertInfosToLoad() const
{
    return m_concertInfosToLoad;
}

QSet<MusicScraperInfo> Storage::musicInfosToLoad() const
{
    return m_musicInfosToLoad;
}

QVector<ImageType> Storage::imageInfosToLoad() const
{
    return m_imageInfosToLoad;
}
