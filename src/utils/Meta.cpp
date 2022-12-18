#include "utils/Meta.h"

#include "data/concert/Concert.h"
#include "data/Image.h"
#include "data/movie/Movie.h"
#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "data/tv_show/TvShow.h"
#include "data/tv_show/TvShowEpisode.h"
#include "model/ImageModel.h"
#include "model/ImageProxyModel.h"
#include "model/music/MusicModelItem.h"
#include "network/DownloadManagerElement.h"

void registerAllMetaTypes()
{
    qRegisterMetaType<DownloadManagerElement>();
    qRegisterMetaType<Image*>("Image*");
    qRegisterMetaType<ImageModel*>("ImageModel*");
    qRegisterMetaType<ImageProxyModel*>("ImageProxyModel*");
    qRegisterMetaType<Album*>("Album*");
    qRegisterMetaType<Artist*>("Artist*");
    qRegisterMetaType<MusicModelItem*>("MusicModelItem*");
    // Required e.g. for DonloadManager
    qRegisterMetaType<Movie*>("Movie*");
    qRegisterMetaType<TvShow*>("TvShow*");
    qRegisterMetaType<TvShowEpisode*>("TvShowEpisode*");
    qRegisterMetaType<Concert*>("Concert*");
}
