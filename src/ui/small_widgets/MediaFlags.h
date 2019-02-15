#pragma once

#include "data/StreamDetails.h"

#include <QWidget>

namespace Ui {
class MediaFlags;
}

/**
 * @brief The MediaFlags class
 *        A widget where flags based on the streamdetails are shown
 */
class MediaFlags : public QWidget
{
    Q_OBJECT

public:
    explicit MediaFlags(QWidget* parent = nullptr);
    ~MediaFlags() override;
    void setStreamDetails(StreamDetails* streamDetails);
    void clear();

private:
    Ui::MediaFlags* ui;
    int m_height{16};

    void setupResolution(StreamDetails* streamDetails);
    void setupAspect(StreamDetails* streamDetails);
    void setupCodec(StreamDetails* streamDetails);
    void setupAudio(StreamDetails* streamDetails);
    void setupChannels(StreamDetails* streamDetails);
    QPixmap colorIcon(QString icon);
};
