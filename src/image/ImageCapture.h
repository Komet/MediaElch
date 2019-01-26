#pragma once

#include "data/StreamDetails.h"

#include <QObject>

class ImageCapture : public QObject
{
    Q_OBJECT
public:
    explicit ImageCapture(QObject *parent = nullptr);
    static bool captureImage(QString file, StreamDetails *streamDetails, QImage &img);
};
