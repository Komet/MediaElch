#pragma once

#include "media/Path.h"

#include <QByteArray>
#include <QImage>

namespace mediaelch {

void resizeBackdrop(QImage& image, bool& resized);
void resizeBackdrop(QByteArray& image);

QImage getImage(mediaelch::FilePath path);

} // namespace mediaelch
