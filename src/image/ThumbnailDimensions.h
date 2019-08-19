#pragma once

namespace mediaelch {

/// @brief Represents the dimensions of a thumbnail, e.g. for episodes.
struct ThumbnailDimensions
{
    int width = 400;
    int height = 300;
};

bool operator==(ThumbnailDimensions lhs, ThumbnailDimensions rhs);

} // namespace mediaelch
