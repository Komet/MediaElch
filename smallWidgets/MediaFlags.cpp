#include "MediaFlags.h"
#include "ui_MediaFlags.h"

#include <QDebug>
#include <QPainter>

#include "../globals/Helper.h"

/**
 * @brief MediaFlags::MediaFlags
 * @param parent
 */
MediaFlags::MediaFlags(QWidget *parent) : QWidget(parent), ui(new Ui::MediaFlags), m_height{16}
{
    ui->setupUi(this);
}

/**
 * @brief MediaFlags::~MediaFlags
 */
MediaFlags::~MediaFlags()
{
    delete ui;
}

void MediaFlags::clear()
{
    ui->mediaFlagResolution->setVisible(false);
    ui->mediaFlagAspect->setVisible(false);
    ui->mediaFlagAudio->setVisible(false);
    ui->mediaFlagChannels->setVisible(false);
    ui->mediaFlagCodec->setVisible(false);
}

/**
 * @brief MediaFlags::setStreamDetails
 * @param streamDetails
 */
void MediaFlags::setStreamDetails(StreamDetails *streamDetails)
{
    setupResolution(streamDetails);
    setupAspect(streamDetails);
    setupCodec(streamDetails);
    setupAudio(streamDetails);
    setupChannels(streamDetails);
}

/**
 * @brief MediaFlags::setupResolution
 * @param streamDetails
 */
void MediaFlags::setupResolution(StreamDetails *streamDetails)
{
    int height = streamDetails->videoDetails().value("height").toInt();
    int width = streamDetails->videoDetails().value("width").toInt();
    QString scanType = streamDetails->videoDetails().value("scantype");
    QString heightFlag = Helper::instance()->matchResolution(width, height, scanType);
    ui->mediaFlagResolution->setVisible(heightFlag != "");
    if (heightFlag != "") {
        ui->mediaFlagResolution->setPixmap(colorIcon(":/media/resolution/" + heightFlag));
    }
}

/**
 * @brief MediaFlags::setupAspect
 * @param streamDetails
 */
void MediaFlags::setupAspect(StreamDetails *streamDetails)
{
    QStringList availableAspects = QStringList() << "1.31"
                                                 << "1.32"
                                                 << "1.33"
                                                 << "1.34"
                                                 << "1.35"
                                                 << "1.36"
                                                 << "1.37"
                                                 << "1.38"
                                                 << "1.66"
                                                 << "1.67"
                                                 << "1.68"
                                                 << "1.77"
                                                 << "1.78"
                                                 << "1.79"
                                                 << "1.82"
                                                 << "1.83"
                                                 << "1.84"
                                                 << "1.85"
                                                 << "1.86"
                                                 << "2.00"
                                                 << "2.14"
                                                 << "2.22"
                                                 << "2.24"
                                                 << "2.33"
                                                 << "2.34"
                                                 << "2.35"
                                                 << "2.36"
                                                 << "2.37"
                                                 << "2.38"
                                                 << "2.39"
                                                 << "2.40"
                                                 << "2.41"
                                                 << "2.42"
                                                 << "2.43"
                                                 << "2.48"
                                                 << "2.50"
                                                 << "2.52"
                                                 << "2.55"
                                                 << "2.73"
                                                 << "2.76";
    double aspect = streamDetails->videoDetails().value("aspect").toDouble();
    QString aspectFlag = QString::number(aspect, 'f', 2);
    ui->mediaFlagAspect->setVisible(availableAspects.contains(aspectFlag));
    if (availableAspects.contains(aspectFlag)) {
        ui->mediaFlagAspect->setPixmap(colorIcon(":/media/aspect/" + aspectFlag));
    }
}

/**
 * @brief MediaFlags::setupCodec
 * @param streamDetails
 */
void MediaFlags::setupCodec(StreamDetails *streamDetails)
{
    QStringList availableCodecs = QStringList() << "avc1"
                                                << "avchd"
                                                << "divx"
                                                << "flv"
                                                << "h264"
                                                << "hevc"
                                                << "mpeg"
                                                << "mpeg1"
                                                << "mpeg2"
                                                << "vc-1"
                                                << "wmv3"
                                                << "xvid";
    QString codec = streamDetails->videoDetails().value("codec").toLower();
    if (codec.startsWith("divx")) {
        codec = "divx";
    }
    if (availableCodecs.contains(codec)) {
        ui->mediaFlagCodec->setPixmap(colorIcon(":/media/codec/" + codec));
    }
    ui->mediaFlagCodec->setVisible(availableCodecs.contains(codec));
}

/**
 * @brief MediaFlags::setupAudio
 * @param streamDetails
 */
void MediaFlags::setupAudio(StreamDetails *streamDetails)
{
    bool visible = false;
    QStringList availableCodecs = QStringList() << "dtshdma"
                                                << "dtshdhra"
                                                << "dtshdx"
                                                << "dolbytruehd"
                                                << "dolbyatmos"
                                                << "dts"
                                                << "dolbydigital"
                                                << "flac"
                                                << "vorbis"
                                                << "mp3"
                                                << "mp2";
    if (streamDetails->audioDetails().count() > 0) {
        QString codec = streamDetails->audioDetails().at(0).value("codec").toLower();
        if (codec == "dtshd-ma" || codec == "dts-hd" || codec == "dtshd_ma") {
            codec = "dtshdma";
        }
        if (codec == "dtshd-hra" || codec == "dtshd_hra") {
            codec = "dtshdhra";
        }
        if (codec == "dtshd-x" || codec == "dtshd_x") {
                    codec = "dtshdx";
        }
        if (codec == "ac3") {
            codec = "dolbydigital";
        }
        if (codec == "atmos") {
                    codec = "dolbyatmos";
        }
        if (availableCodecs.contains(codec)) {
            ui->mediaFlagAudio->setPixmap(colorIcon(":/media/audio/" + codec));
            visible = true;
        }
    }
    ui->mediaFlagAudio->setVisible(visible);
}

/**
 * @brief MediaFlags::setupChannels
 * @param streamDetails
 */
void MediaFlags::setupChannels(StreamDetails *streamDetails)
{
    int channels = -1;
    for (int i = 0, n = streamDetails->audioDetails().count(); i < n; ++i) {
        if (streamDetails->audioDetails().at(i).value("channels").toInt() > channels) {
            channels = streamDetails->audioDetails().at(i).value("channels").toInt();
        }
    }

    if (channels > 8 || channels < 2 || channels == 3 || channels == 4) {
        channels = -1;
    }

    if (channels != -1) {
        ui->mediaFlagChannels->setPixmap(colorIcon(QString(":/media/channels/%1").arg(channels)));
    }
    ui->mediaFlagChannels->setVisible(channels != -1);
}

QPixmap MediaFlags::colorIcon(QString icon)
{
    static QMap<QString, QPixmap> pixmaps;
    if (pixmaps.contains(icon)) {
        return pixmaps.value(icon);
    }

    QPixmap pixmap =
        QPixmap(icon).scaledToHeight(m_height * Helper::instance()->devicePixelRatio(this), Qt::SmoothTransformation);
    Helper::instance()->setDevicePixelRatio(pixmap, Helper::instance()->devicePixelRatio(this));
    QPainter p;
    p.begin(&pixmap);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(pixmap.rect(), QColor(34, 79, 127, 255));
    p.end();
    pixmaps.insert(icon, pixmap);
    return pixmap;
}
