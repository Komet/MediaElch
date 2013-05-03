#include "Storage.h"

Storage::Storage(QObject *parent, Movie *movie) :
    QObject(parent)
{
    m_movie = QPointer<Movie>(movie);
}

Storage::Storage(QObject *parent, Concert *concert) :
    QObject(parent)
{
    m_concert = QPointer<Concert>(concert);
}

Storage::Storage(QObject *parent, TvShow *show) :
    QObject(parent)
{
    m_show = QPointer<TvShow>(show);
}

Storage::Storage(QObject *parent, TvShowEpisode *episode) :
    QObject(parent)
{
    m_episode = QPointer<TvShowEpisode>(episode);
}

Storage::Storage(QObject *parent, QList<ScraperSearchResult> results) :
    QObject(parent)
{
    m_results = results;
}

Storage::Storage(QObject *parent, QList<int> infosToLoad) :
    QObject(parent)
{
    m_infosToLoad = infosToLoad;
}

Storage::Storage(QObject *parent, ExportTemplate *exportTemplate) :
    QObject(parent)
{
    m_exportTemplate = exportTemplate;
}

Storage::Storage(QObject *parent, QMap<ScraperInterface *, QString> ids) :
    QObject(parent)
{
    m_ids = ids;
}

Movie *Storage::movie()
{
    if (m_movie)
        return m_movie;
    return 0;
}

Concert *Storage::concert()
{
    if (m_concert)
        return m_concert;
    return 0;
}

TvShow *Storage::show()
{
    if (m_show)
        return m_show;
    return 0;
}

TvShowEpisode *Storage::episode()
{
    if (m_episode)
        return m_episode;
    return 0;
}

ExportTemplate *Storage::exportTemplate()
{
    if (m_exportTemplate)
        return m_exportTemplate;
    return 0;
}

QMap<ScraperInterface*, QString> Storage::ids()
{
    return m_ids;
}

QVariant Storage::toVariant(QObject *parent, Movie *movie)
{
    Storage *storage = new Storage(parent, movie);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, Concert *concert)
{
    Storage *storage = new Storage(parent, concert);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, TvShow *show)
{
    Storage *storage = new Storage(parent, show);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, TvShowEpisode *episode)
{
    Storage *storage = new Storage(parent, episode);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QList<ScraperSearchResult> results)
{
    Storage *storage = new Storage(parent, results);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QList<int> infosToLoad)
{
    Storage *storage = new Storage(parent, infosToLoad);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, ExportTemplate *exportTemplate)
{
    Storage *storage = new Storage(parent, exportTemplate);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QMap<ScraperInterface *, QString> ids)
{
    Storage *storage = new Storage(parent, ids);
    QVariant var;
    var.setValue(storage);
    return var;
}

QList<ScraperSearchResult> Storage::results()
{
    return m_results;
}

QList<int> Storage::infosToLoad()
{
    return m_infosToLoad;
}
