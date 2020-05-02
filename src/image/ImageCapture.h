#pragma once

#include "data/StreamDetails.h"
#include "file/Path.h"
#include "image/ThumbnailDimensions.h"

#include <QObject>

namespace mediaelch {

class ImageCapture : public QObject
{
    Q_OBJECT

public:
    explicit ImageCapture(QObject* parent = nullptr);
    /// @brief Captures a screenshot of a given video file at a random time.
    ///
    /// Resizes it to the given dimension with respect to its aspect ratio.
    /// If one dimension is 0 then no scaling is performed at all.
    /// If cropFromCenter is true then the image will be resized to *exactly*
    /// the given dimensions and will crop a rectangle from the screenshot's
    /// center. Otherwise the resulting image may be smaller in size or height
    /// than the given dimensions.
    static bool captureImage(FilePath file,
        StreamDetails* streamDetails,
        ThumbnailDimensions dim,
        QImage& img,
        bool cropFromCenter = false);
};

} // namespace mediaelch
