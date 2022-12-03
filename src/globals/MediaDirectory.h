#pragma once

#include <QDir>

namespace mediaelch {

struct MediaDirectory
{
    QDir path;
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
