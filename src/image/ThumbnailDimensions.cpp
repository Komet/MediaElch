#include "image/ThumbnailDimensions.h"

namespace mediaelch {

bool operator==(ThumbnailDimensions lhs, ThumbnailDimensions rhs)
{
    return lhs.width == rhs.width && lhs.height == rhs.height;
}

} // namespace mediaelch
