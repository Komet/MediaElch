#ifndef DOWNLOADMANAGERELEMENT_H
#define DOWNLOADMANAGERELEMENT_H

#include "globals/Globals.h"

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
    ImageType imageType;
    QUrl url;
    QImage image;
    qint64 bytesReceived;
    qint64 bytesTotal;
    Actor *actor;
    TvShowEpisode *episode;
    Movie *movie;
    TvShow *show;
    Concert *concert;
    int season;
};

#endif // DOWNLOADMANAGERELEMENT_H
