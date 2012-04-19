#include "TvShowEpisode.h"

TvShowEpisode::TvShowEpisode(QStringList files, TvShow *parent) :
    QObject(parent)
{
    m_files = files;
    m_parent = parent;
}

QString TvShowEpisode::name()
{
    return m_files.last().split("/").last();
}
