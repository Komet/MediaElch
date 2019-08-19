#pragma once

#include "data/StreamDetails.h"
#include "image/ThumbnailDimensions.h"

#include <QObject>

namespace mediaelch {

class ImageCapture : public QObject
{
    Q_OBJECT

public:
    explicit ImageCapture(QObject* parent = nullptr);
    /// @brief Captures a screenshot of a given video file at a random time.
    ///        Resizes it to the given dimension with respect to its aspect ratio.
    static bool captureImage(QString file, StreamDetails* streamDetails, ThumbnailDimensions dim, QImage& img);
};

} // namespace mediaelch
