#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include "data/MediaCenterInterface.h"
#include "data/MovieFileSearcher.h"
#include "data/ScraperInterface.h"
#include "data/TvScraperInterface.h"
#include "data/TvShowFileSearcher.h"
#include "data/MovieModel.h"
#include "data/TvShowModel.h"
#include "data/TvShowProxyModel.h"

class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager(QObject *parent = 0);
    
    static Manager *instance();
    QList<MediaCenterInterface*> mediaCenters();
    QList<ScraperInterface*> scrapers();
    QList<TvScraperInterface*> tvScrapers();
    MediaCenterInterface *mediaCenterInterface();
    MovieFileSearcher* movieFileSearcher();
    TvShowFileSearcher* tvShowFileSearcher();
    MovieModel* movieModel();
    TvShowModel* tvShowModel();
    TvShowProxyModel *tvShowProxyModel();

private:
    QList<MediaCenterInterface*> m_mediaCenters;
    QList<ScraperInterface*> m_scrapers;
    QList<TvScraperInterface*> m_tvScrapers;
    MovieFileSearcher* m_movieFileSearcher;
    TvShowFileSearcher* m_tvShowFileSearcher;
    MovieModel* m_movieModel;
    TvShowModel* m_tvShowModel;
    TvShowProxyModel* m_tvShowProxyModel;
};

#endif // MANAGER_H
