#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include "data/MediaCenterInterface.h"
#include "data/MovieFileSearcher.h"
#include "data/ScraperInterface.h"
#include "data/MovieModel.h"
#include "mediaCenterPlugins/XbmcXml.h"
#include "scrapers/TMDb.h"
#include "scrapers/VideoBuster.h"

class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager(QObject *parent = 0);
    
    static Manager *instance();
    QList<MediaCenterInterface*> mediaCenters();
    QList<ScraperInterface*> scrapers();
    MediaCenterInterface *mediaCenterInterface();
    MovieFileSearcher* movieFileSearcher();
    MovieModel* movieModel();

private:
    QList<MediaCenterInterface*> m_mediaCenters;
    QList<ScraperInterface*> m_scrapers;
    MovieFileSearcher* m_movieFileSearcher;
    MovieModel* m_movieModel;
};

#endif // MANAGER_H
