#ifndef IMAGECAPTURE_H
#define IMAGECAPTURE_H

#include <QObject>
#include "data/StreamDetails.h"

class ImageCapture : public QObject
{
    Q_OBJECT
public:
    explicit ImageCapture(QObject *parent = 0);
    static bool captureImage(QString file, StreamDetails *streamDetails, QImage &img);
};

#endif // IMAGECAPTURE_H
