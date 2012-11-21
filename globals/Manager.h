#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include "settings/Settings.h"
#include "data/ConcertFileSearcher.h"
#include "data/ConcertModel.h"
#include "data/ConcertScraperInterface.h"
#include "data/ImageProviderInterface.h"
#include "data/MediaCenterInterface.h"
#include "data/MovieFileSearcher.h"
#include "data/ScraperInterface.h"
#include "data/TvScraperInterface.h"
#include "data/TvShowFileSearcher.h"
#include "data/MovieModel.h"
#include "data/TvShowModel.h"
#include "data/TvShowProxyModel.h"
#include "imageProviders/FanartTv.h"

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
    QList<ConcertScraperInterface*> concertScrapers();
    QList<ImageProviderInterface*> imageProviders();
    QList<ImageProviderInterface*> imageProviders(int type);
    MediaCenterInterface *mediaCenterInterface();
    MediaCenterInterface *mediaCenterInterfaceTvShow();
    MediaCenterInterface *mediaCenterInterfaceConcert();
    MovieFileSearcher* movieFileSearcher();
    TvShowFileSearcher* tvShowFileSearcher();
    ConcertFileSearcher* concertFileSearcher();
    MovieModel* movieModel();
    TvShowModel* tvShowModel();
    TvShowProxyModel *tvShowProxyModel();
    ConcertModel* concertModel();
    void setupMediaCenterInterface();
    void shutdownMediaCenterInterfaces();
    ScraperInterface* getScraperForName(QString name);
    QSqlDatabase cacheDb();
    void clearCacheDatabase();
    FanartTv* fanartTv();

private:
    QList<MediaCenterInterface*> m_mediaCenters;
    QList<MediaCenterInterface*> m_mediaCentersTvShow;
    QList<MediaCenterInterface*> m_mediaCentersConcert;
    QList<ScraperInterface*> m_scrapers;
    QList<TvScraperInterface*> m_tvScrapers;
    QList<ConcertScraperInterface*> m_concertScrapers;
    QList<ImageProviderInterface*> m_imageProviders;
    MovieFileSearcher* m_movieFileSearcher;
    TvShowFileSearcher* m_tvShowFileSearcher;
    ConcertFileSearcher* m_concertFileSearcher;
    MovieModel* m_movieModel;
    TvShowModel* m_tvShowModel;
    TvShowProxyModel* m_tvShowProxyModel;
    ConcertModel* m_concertModel;
    Settings *m_settings;
    QSqlDatabase *m_cacheDb;
    void setupCacheDatabase();
    void closeCacheDatabase();
};

#endif // MANAGER_H
