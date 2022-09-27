#include "utils/Meta.h"

#include "data/Image.h"
#include "data/music/Album.h"
#include "data/music/Artist.h"
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
}
