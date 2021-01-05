#include "globals/Meta.h"

#include "globals/DownloadManagerElement.h"
#include "image/Image.h"
#include "image/ImageModel.h"
#include "image/ImageProxyModel.h"
#include "music/Album.h"
#include "music/Artist.h"
#include "music/MusicModelItem.h"

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
