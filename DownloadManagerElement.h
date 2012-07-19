#ifndef DOWNLOADMANAGERELEMENT_H
#define DOWNLOADMANAGERELEMENT_H

#include "Globals.h"

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
    int season;
};

#endif // DOWNLOADMANAGERELEMENT_H
