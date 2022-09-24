#pragma once

#include <QImage>
#include <QString>
#include <QUrl>

struct TrailerResult
{
    QUrl preview;
    QString name;
    QString language;
    QUrl trailerUrl;
    QImage previewImage;
    bool previewImageLoaded{false};
};
