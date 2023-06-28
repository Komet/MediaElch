#pragma once

#include "media/Path.h"

namespace mediaelch {

struct MediaDirectory
{
    DirectoryPath path;
    bool separateFolders = false;
    bool autoReload = false;
    bool disabled = false;
};

enum class MediaDirectoryType : int
{
    Movies,
    TvShows,
    Concerts,
    Downloads,
    Music
};

} // namespace mediaelch
