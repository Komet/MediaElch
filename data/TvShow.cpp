#include "TvShow.h"
#include "Globals.h"

#include <QDebug>

TvShow::TvShow(QString dir, QObject *parent) :
    QObject(parent)
{
    m_dir = dir;
}

void TvShow::addEpisode(TvShowEpisode *episode)
{
    m_episodes.append(episode);
}

QString TvShow::name()
{
    return m_dir.split("/").last();
}

int TvShow::episodeCount()
{
    return m_episodes.count();
}
