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
#include "plugins/PluginInterface.h"
#include "plugins/PluginManager.h"

class Storage : public QObject
{
    Q_OBJECT
public:
    explicit Storage(QObject *parent, Movie *movie);
    explicit Storage(QObject *parent, Concert *concert);
    explicit Storage(QObject *parent, TvShow *show);
    explicit Storage(QObject *parent, TvShowEpisode *episode);
    explicit Storage(QObject *parent, QList<ScraperSearchResult> results);
    explicit Storage(QObject *parent, QList<int> infosToLoad);
    explicit Storage(QObject *parent, ExportTemplate *exportTemplate);
    explicit Storage(QObject *parent, QMap<ScraperInterface*, QString> ids);
    explicit Storage(QObject *parent, QTableWidgetItem *item);
    explicit Storage(QObject *parent, PluginInterface *pluginInterface);
    explicit Storage(QObject *parent, PluginManager::Plugin plugin);
    Movie *movie();
    Concert *concert();
    TvShow *show();
    TvShowEpisode *episode();
    QList<ScraperSearchResult> results();
    QList<int> infosToLoad();
    ExportTemplate *exportTemplate();
    QMap<ScraperInterface*, QString> ids();
    QTableWidgetItem *tableWidgetItem();
    PluginInterface *pluginInterface();
    PluginManager::Plugin plugin();
    static QVariant toVariant(QObject *parent, Movie *movie);
    static QVariant toVariant(QObject *parent, Concert *concert);
    static QVariant toVariant(QObject *parent, TvShow *show);
    static QVariant toVariant(QObject *parent, TvShowEpisode *episode);
    static QVariant toVariant(QObject *parent, QList<ScraperSearchResult> results);
    static QVariant toVariant(QObject *parent, QList<int> infosToLoad);
    static QVariant toVariant(QObject *parent, ExportTemplate *exportTemplate);
    static QVariant toVariant(QObject *parent, QMap<ScraperInterface*, QString> ids);
    static QVariant toVariant(QObject *parent, QTableWidgetItem *item);
    static QVariant toVariant(QObject *parent, PluginInterface *pluginInterface);
    static QVariant toVariant(QObject *parent, PluginManager::Plugin plugin);

private:
    QPointer<Movie> m_movie;
    QPointer<Concert> m_concert;
    QPointer<TvShow> m_show;
    QPointer<TvShowEpisode> m_episode;
    QList<ScraperSearchResult> m_results;
    QList<int> m_infosToLoad;
    QPointer<ExportTemplate> m_exportTemplate;
    QMap<ScraperInterface*, QString> m_ids;
    QTableWidgetItem *m_tableWidgetItem;
    PluginInterface *m_pluginInterface;
    PluginManager::Plugin m_plugin;
};

Q_DECLARE_METATYPE(Storage*)

#endif // STORAGE_H
