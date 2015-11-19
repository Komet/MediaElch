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

Storage::Storage(QObject *parent, Artist *artist) :
    QObject(parent)
{
    m_artist = QPointer<Artist>(artist);
}

Storage::Storage(QObject *parent, Album *album) :
    QObject(parent)
{
    m_album = QPointer<Album>(album);
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

Storage::Storage(QObject *parent, QTableWidgetItem *item) :
    QObject(parent)
{
    m_tableWidgetItem = item;
}

Storage::Storage(QObject *parent, PluginInterface *plugin) :
    QObject(parent)
{
    m_pluginInterface = plugin;
}

Storage::Storage(QObject *parent, PluginManager::Plugin plugin) :
    QObject(parent)
{
    m_plugin = plugin;
}

Storage::Storage(QObject *parent, QList<TvShowEpisode *> episodes) :
    QObject(parent)
{
    m_episodes = episodes;
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

Artist *Storage::artist()
{
    if (m_artist)
        return m_artist;
    return 0;
}

Album *Storage::album()
{
    if (m_album)
        return m_album;
    return 0;
}

ExportTemplate *Storage::exportTemplate()
{
    if (m_exportTemplate)
        return m_exportTemplate;
    return 0;
}

QTableWidgetItem *Storage::tableWidgetItem()
{
    if (m_tableWidgetItem)
        return m_tableWidgetItem;
    return 0;
}

QMap<ScraperInterface*, QString> Storage::ids()
{
    return m_ids;
}

PluginInterface *Storage::pluginInterface()
{
    return m_pluginInterface;
}

PluginManager::Plugin Storage::plugin()
{
    return m_plugin;
}

QList<TvShowEpisode*> Storage::episodes()
{
    return m_episodes;
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

QVariant Storage::toVariant(QObject *parent, Artist *artist)
{
    Storage *storage = new Storage(parent, artist);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, Album *album)
{
    Storage *storage = new Storage(parent, album);
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

QVariant Storage::toVariant(QObject *parent, QTableWidgetItem *item)
{
    Storage *storage = new Storage(parent, item);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, PluginInterface *plugin)
{
    Storage *storage = new Storage(parent, plugin);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, PluginManager::Plugin plugin)
{
    Storage *storage = new Storage(parent, plugin);
    QVariant var;
    var.setValue(storage);
    return var;
}

QVariant Storage::toVariant(QObject *parent, QList<TvShowEpisode*> episodes)
{
    Storage *storage = new Storage(parent, episodes);
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
