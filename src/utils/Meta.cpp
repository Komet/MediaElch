#include "utils/Meta.h"

#include "data/Image.h"
#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "data/tv_show/TvShow.h"
#include "data/tv_show/TvShowEpisode.h"
#include "model/ImageModel.h"
#include "model/ImageProxyModel.h"
#include "model/music/MusicModelItem.h"
#include "network/DownloadManagerElement.h"

#include <iostream>

#ifdef MEDIAELCH_USE_ASAN_STACKTRACE

// ASAN has a function to pretty-print a stack trace.  Something _really_ helpful!
// See <https://github.com/llvm/llvm-project/blob/main/compiler-rt/include/sanitizer/common_interface_defs.h>
extern "C" {
void __sanitizer_print_stack_trace(void);
}

namespace mediaelch {
namespace internal {


void mediaelch_print_stacktrace()
{
    __sanitizer_print_stack_trace();
    std::cout.flush();
    std::cerr.flush();
}

} // namespace internal
} // namespace mediaelch

#endif

void registerAllMetaTypes()
{
    qRegisterMetaType<DownloadManagerElement>();
    qRegisterMetaType<Image*>("Image*");
    qRegisterMetaType<ImageModel*>("ImageModel*");
    qRegisterMetaType<ImageProxyModel*>("ImageProxyModel*");
    qRegisterMetaType<Album*>("Album*");
    qRegisterMetaType<Artist*>("Artist*");
    qRegisterMetaType<MusicModelItem*>("MusicModelItem*");
    // Required e.g. for DownloadManager
    qRegisterMetaType<Movie*>("Movie*");
    qRegisterMetaType<TvShow*>("TvShow*");
    qRegisterMetaType<TvShowEpisode*>("TvShowEpisode*");
    qRegisterMetaType<Concert*>("Concert*");
}
