#pragma once

#include <Qt>

namespace mediaelch {

enum class MusicType : int
{
    None,
    Artist,
    Album
};

namespace MusicRoles {
const int Type = Qt::UserRole + 1;
const int IsNew = Qt::UserRole + 2;
const int HasChanged = Qt::UserRole + 3;
const int NumOfAlbums = Qt::UserRole + 4;
const int SelectionForeground = Qt::UserRole + 5;
} // namespace MusicRoles

} // namespace mediaelch
