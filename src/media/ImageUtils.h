#pragma once

#include "media/Path.h"

#include <QByteArray>
#include <QImage>

namespace mediaelch {

void resizeBackdrop(QImage& image, bool& resized);
void resizeBackdrop(QByteArray& image);

QImage getImage(mediaelch::FilePath path);

QImage scaledImage(const QImage& img, int width, int height);
QImage scaledImage(const QImage& img, QSize size);

} // namespace mediaelch
