#include "Manager.h"

#include <QApplication>

Manager::Manager(QObject *parent) :
    QObject(parent)
{
    m_mediaCenters.append(new XbmcXml(this));
    m_scrapers.append(new TMDb(this));
    m_movieFileSearcher = new MovieFileSearcher(this);
    m_movieModel = new MovieModel(this);
}

Manager* Manager::instance()
{
    static Manager *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new Manager(qApp);
    }
    return m_instance;
}

MediaCenterInterface *Manager::mediaCenterInterface()
{
    return m_mediaCenters.at(0);
}

MovieFileSearcher *Manager::movieFileSearcher()
{
    return m_movieFileSearcher;
}

QList<ScraperInterface*> Manager::scrapers()
{
    return m_scrapers;
}

MovieModel *Manager::movieModel()
{
    return m_movieModel;
}
