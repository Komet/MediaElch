#ifndef DOWNLOADMANAGERELEMENT_H
#define DOWNLOADMANAGERELEMENT_H

#include "globals/Globals.h"

class Album;
class Artist;
class Concert;
class Movie;
class TvShow;
class TvShowEpisode;

/**
 * @brief The DownloadManagerElement class
 */
class DownloadManagerElement
{
public:
    DownloadManagerElement();
    int imageType;
    QUrl url;
    QByteArray data;
    qint64 bytesReceived;
    qint64 bytesTotal;
    Actor *actor;
    TvShowEpisode *episode;
    Movie *movie;
    TvShow *show;
    Concert *concert;
    Album *album;
    Artist *artist;
    int season;
    bool directDownload;
};

#endif // DOWNLOADMANAGERELEMENT_H
