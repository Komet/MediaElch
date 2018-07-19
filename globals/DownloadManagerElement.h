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
    ImageType imageType{ImageType::None};
    QUrl url;
    QByteArray data;
    qint64 bytesReceived{0};
    qint64 bytesTotal{0};
    Actor *actor{nullptr};
    TvShowEpisode *episode{nullptr};
    Movie *movie{nullptr};
    TvShow *show{nullptr};
    Concert *concert{nullptr};
    Album *album{nullptr};
    Artist *artist{nullptr};
    int season{0};
    bool directDownload{false};
};

#endif // DOWNLOADMANAGERELEMENT_H
