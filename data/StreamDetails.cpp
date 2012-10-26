#include "StreamDetails.h"

#include <QApplication>
#include "MediaInfo/MediaInfo.h"

using namespace MediaInfoLib;

StreamDetails::StreamDetails(QObject *parent, QString file) :
    QObject(parent)
{
    m_file = file;
}

void StreamDetails::clear()
{
    m_videoDetails.clear();
    m_audioDetails.clear();
    m_subtitles.clear();
}

void StreamDetails::loadStreamDetails()
{
    clear();

    MediaInfo MI;
    MI.Option(QString("Info_Version").toStdWString(), QString("0.7.60;%1;%2").arg(QApplication::applicationName()).arg(QApplication::applicationVersion()).toStdWString());
    MI.Option(QString("Internet").toStdWString(), QString("no").toStdWString());
    MI.Open(m_file.toStdWString());
    MI.Option(QString("Complete").toStdWString(), QString("1").toStdWString());

    int duration = 0;
    double aspectRatio;
    int width = 0;
    int height = 0;
    QString scanType;
    QString videoCodec;

    int videoCount = QString::fromStdWString(MI.Get(Stream_General, 0, QString("VideoCount").toStdWString())).toInt();
    int audioCount = QString::fromStdWString(MI.Get(Stream_General, 0, QString("AudioCount").toStdWString())).toInt();
    int textCount = QString::fromStdWString(MI.Get(Stream_General, 0, QString("TextCount").toStdWString())).toInt();

    duration = QString::fromStdWString(MI.Get(Stream_General, 0, QString("Duration").toStdWString())).toInt()/1000;
    setVideoDetail("durationinseconds", QString("%1").arg(duration));

    if (videoCount > 0) {
        aspectRatio = QString::fromStdWString(MI.Get(Stream_Video, 0, QString("DisplayAspectRatio").toStdWString())).toDouble();
        width = QString::fromStdWString(MI.Get(Stream_Video, 0, QString("Width").toStdWString())).toInt();
        height = QString::fromStdWString(MI.Get(Stream_Video, 0, QString("Height").toStdWString())).toInt();
        scanType = QString::fromStdWString(MI.Get(Stream_Video, 0, QString("ScanType").toStdWString())).toLower();

        QString codec = QString::fromStdWString(MI.Get(Stream_Video, 0, QString("CodecID/Hint").toStdWString()));
        QString version;
        if (codec.isEmpty()) {
            codec = QString::fromStdWString(MI.Get(Stream_Video, 0, QString("CodecID").toStdWString()));
            if (codec.isEmpty()) {
                codec = QString::fromStdWString(MI.Get(Stream_Video, 0, QString("Format").toStdWString()));
                version = QString::fromStdWString(MI.Get(Stream_Video, 0, QString("Format_Version").toStdWString()));
            }
        }
        videoCodec = videoFormat(codec, version);

        setVideoDetail("codec", videoCodec);
        setVideoDetail("aspect", QString("%1").arg(aspectRatio));
        setVideoDetail("width", QString("%1").arg(width));
        setVideoDetail("height", QString("%1").arg(height));
        setVideoDetail("scantype", scanType);
    }

    for (int i=0 ; i<audioCount ; ++i) {
        QString lang = QString::fromStdWString(MI.Get(Stream_Audio, i, QString("Language/String3").toStdWString())).toLower();
        QString audioCodec = audioFormat(QString::fromStdWString(MI.Get(Stream_Audio, i, QString("CodecID").toStdWString())));
        int channels = QString::fromStdWString(MI.Get(Stream_Audio, i, QString("Channels").toStdWString())).toInt();
        setAudioDetail(i, "language", lang);
        setAudioDetail(i, "codec", audioCodec);
        setAudioDetail(i, "channels", QString("%1").arg(channels));
    }

    for (int i=0 ; i<textCount ; ++i) {
        QString lang = QString::fromStdWString(MI.Get(Stream_Text, i, QString("Language/String3").toStdWString())).toLower();
        setSubtitleDetail(i, "language", lang);
    }
    MI.Close();
}

QString StreamDetails::videoFormat(QString format, QString version)
{
    format = format.toLower();
    if (!format.isEmpty() && format == "mpeg video")
        format = (version.toLower() == "version 2") ? "mpeg2" : "mpeg";
    if (format == "v_mpeg4/iso/avc")
        format = "avc";
    return format;
}

QString StreamDetails::audioFormat(QString format)
{
    format = format.toLower();
    if (format == "dts-hd")
        format = "dtshd_ma";
    return format;
}

void StreamDetails::setVideoDetail(QString key, QString value)
{
    m_videoDetails.insert(key, value);
}

void StreamDetails::setAudioDetail(int streamNumber, QString key, QString value)
{
    if (streamNumber >= m_audioDetails.count())
        m_audioDetails.insert(streamNumber, QMap<QString, QString>());
    if (streamNumber >= m_audioDetails.count())
        return;
    m_audioDetails[streamNumber].insert(key, value);
}

void StreamDetails::setSubtitleDetail(int streamNumber, QString key, QString value)
{
    if (streamNumber >= m_subtitles.count())
        m_subtitles.insert(streamNumber, QMap<QString, QString>());
    if (streamNumber >= m_subtitles.count())
        return;
    m_subtitles[streamNumber].insert(key, value);
}

QMap<QString, QString> StreamDetails::videoDetails()
{
    return m_videoDetails;
}

QList<QMap<QString, QString> > StreamDetails::audioDetails()
{
    return m_audioDetails;
}

QList<QMap<QString, QString> > StreamDetails::subtitleDetails()
{
    return m_subtitles;
}
