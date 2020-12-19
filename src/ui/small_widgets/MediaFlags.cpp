#include "MediaFlags.h"
#include "ui_MediaFlags.h"

#include <QDebug>
#include <QPainter>

#include "globals/Helper.h"

MediaFlags::MediaFlags(QWidget* parent) : QWidget(parent), ui(new Ui::MediaFlags)
{
    ui->setupUi(this);
}

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

void MediaFlags::setStreamDetails(StreamDetails* streamDetails)
{
    setupResolution(streamDetails);
    setupAspect(streamDetails);
    setupCodec(streamDetails);
    setupAudio(streamDetails);
    setupChannels(streamDetails);
}

void MediaFlags::setupResolution(StreamDetails* streamDetails)
{
    const auto videoDetails = streamDetails->videoDetails();
    const int height = videoDetails.value(StreamDetails::VideoDetails::Height).toInt();
    const int width = videoDetails.value(StreamDetails::VideoDetails::Width).toInt();
    QString scanType = videoDetails.value(StreamDetails::VideoDetails::ScanType);
    QString heightFlag = helper::matchResolution(width, height, scanType);
    ui->mediaFlagResolution->setVisible(heightFlag != "");
    if (heightFlag != "") {
        ui->mediaFlagResolution->setPixmap(colorIcon(":/media/resolution/" + heightFlag));
    }
}

void MediaFlags::setupAspect(StreamDetails* streamDetails)
{
    QStringList availableAspects = {"1.31",
        "1.32",
        "1.33",
        "1.34",
        "1.35",
        "1.36",
        "1.37",
        "1.38",
        "1.66",
        "1.67",
        "1.68",
        "1.77",
        "1.78",
        "1.79",
        "1.82",
        "1.83",
        "1.84",
        "1.85",
        "1.86",
        "2.00",
        "2.14",
        "2.22",
        "2.24",
        "2.33",
        "2.34",
        "2.35",
        "2.36",
        "2.37",
        "2.38",
        "2.39",
        "2.40",
        "2.41",
        "2.42",
        "2.43",
        "2.48",
        "2.50",
        "2.52",
        "2.55",
        "2.73",
        "2.76"};
    const double aspect = streamDetails->videoDetails().value(StreamDetails::VideoDetails::Aspect).toDouble();
    QString aspectFlag = QString::number(aspect, 'f', 2);
    ui->mediaFlagAspect->setVisible(availableAspects.contains(aspectFlag));
    if (availableAspects.contains(aspectFlag)) {
        ui->mediaFlagAspect->setPixmap(colorIcon(":/media/aspect/" + aspectFlag));
    }
}

void MediaFlags::setupCodec(StreamDetails* streamDetails)
{
    QStringList availableCodecs = {
        "avc1", "avchd", "divx", "flv", "h264", "avc", "av1", "hevc", "mpeg", "mpeg1", "mpeg2", "vc-1", "wmv3", "xvid"};
    QString codec = streamDetails->videoDetails().value(StreamDetails::VideoDetails::Codec).toLower();
    if (codec.startsWith("divx")) {
        codec = "divx";
    }
    if (availableCodecs.contains(codec)) {
        ui->mediaFlagCodec->setPixmap(colorIcon(":/media/codec/" + codec));
    }
    ui->mediaFlagCodec->setVisible(availableCodecs.contains(codec));
}

void MediaFlags::setupAudio(StreamDetails* streamDetails)
{
    bool visible = false;
    QStringList availableCodecs = {"dtshdma",
        "dtshdhra",
        "dtshdx",
        "dolbytruehd",
        "dolbyatmos",
        "dts",
        "dolbydigital",
        "dolbydigitalplus",
        "flac",
        "vorbis",
        "mp3",
        "mp2",
        "aac",
        "aac lc"};
    if (streamDetails->audioDetails().count() > 0) {
        QString codec = streamDetails->audioDetails().at(0).value(StreamDetails::AudioDetails::Codec).toLower();
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
        if (codec == "eac3") {
            codec = "dolbydigitalplus";
        }
        if (codec == "atmos") {
            codec = "dolbyatmos";
        }
        if (codec == "truehd") {
            codec = "dolbytruehd";
        }
        if (codec == "aac" || codec == "aac lc") {
            codec = "aac";
        }
        if (availableCodecs.contains(codec)) {
            ui->mediaFlagAudio->setPixmap(colorIcon(":/media/audio/" + codec));
            visible = true;
        }
    }
    ui->mediaFlagAudio->setVisible(visible);
}

void MediaFlags::setupChannels(StreamDetails* streamDetails)
{
    int channels = -1;
    for (int i = 0, n = streamDetails->audioDetails().count(); i < n; ++i) {
        if (streamDetails->audioDetails().at(i).value(StreamDetails::AudioDetails::Channels).toInt() > channels) {
            channels = streamDetails->audioDetails().at(i).value(StreamDetails::AudioDetails::Channels).toInt();
        }
    }

    if (channels > 8 || channels < 2 || channels == 3 || channels == 4) {
        channels = -1;
    }

    if (channels != -1) {
        ui->mediaFlagChannels->setPixmap(colorIcon(QStringLiteral(":/media/channels/%1").arg(channels)));
    }
    ui->mediaFlagChannels->setVisible(channels != -1);
}

QPixmap MediaFlags::colorIcon(QString icon)
{
    static QMap<QString, QPixmap> pixmaps;
    if (pixmaps.contains(icon)) {
        return pixmaps.value(icon);
    }

    const int height = static_cast<int>(m_height * helper::devicePixelRatio(this));
    QPixmap pixmap = QPixmap(icon).scaledToHeight(height, Qt::SmoothTransformation);
    helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
    QPainter p;
    p.begin(&pixmap);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(pixmap.rect(), QColor(34, 79, 127, 255));
    p.end();
    pixmaps.insert(icon, pixmap);
    return pixmap;
}
