#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include "SettingsWidget.h"
#include "data/MediaCenterInterface.h"
#include "data/MovieFileSearcher.h"
#include "data/ScraperInterface.h"
#include "data/TvScraperInterface.h"
#include "data/TvShowFileSearcher.h"
#include "data/MovieModel.h"
#include "data/TvShowModel.h"
#include "data/TvShowProxyModel.h"

/**
 * @brief The Manager class
 * This class handles the various interfaces
 */
class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager(QObject *parent = 0);
    ~Manager();

    static Manager *instance();
    QList<MediaCenterInterface*> mediaCenters();
    QList<ScraperInterface*> scrapers();
    QList<TvScraperInterface*> tvScrapers();
    MediaCenterInterface *mediaCenterInterface();
    MediaCenterInterface *mediaCenterInterfaceTvShow();
    MovieFileSearcher* movieFileSearcher();
    TvShowFileSearcher* tvShowFileSearcher();
    MovieModel* movieModel();
    TvShowModel* tvShowModel();
    TvShowProxyModel *tvShowProxyModel();
    void setupMediaCenterInterface();
    void shutdownMediaCenterInterfaces();

private:
    QList<MediaCenterInterface*> m_mediaCenters;
    QList<MediaCenterInterface*> m_mediaCentersTvShow;
    QList<ScraperInterface*> m_scrapers;
    QList<TvScraperInterface*> m_tvScrapers;
    MovieFileSearcher* m_movieFileSearcher;
    TvShowFileSearcher* m_tvShowFileSearcher;
    MovieModel* m_movieModel;
    TvShowModel* m_tvShowModel;
    TvShowProxyModel* m_tvShowProxyModel;
    SettingsWidget *m_settingsWidget;
};

#endif // MANAGER_H
