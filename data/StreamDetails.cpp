#include "StreamDetails.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include "MediaInfo/MediaInfo.h"
#include "settings/Settings.h"

using namespace MediaInfoLib;

/**
 * @brief StreamDetails::StreamDetails
 * @param parent
 * @param file
 */
StreamDetails::StreamDetails(QObject *parent, QStringList files) :
    QObject(parent)
{
    m_files = files;
}

/**
 * @brief Clears all information
 */
void StreamDetails::clear()
{
    m_videoDetails.clear();
    m_audioDetails.clear();
    m_subtitles.clear();
}

/**
 * @brief Loads stream details from the file
 */
void StreamDetails::loadStreamDetails()
{
    clear();
    if (m_files.isEmpty())
        return;

    if (m_files.first().endsWith(".iso", Qt::CaseInsensitive) || m_files.first().endsWith(".img", Qt::CaseInsensitive))
        return;

    // If it's a DVD structure, compute the biggest part (main movie) and use this IFO file
    if (m_files.first().endsWith("VIDEO_TS.IFO")) {
        QMap<QString, qint64> sizes;
        QString biggest;
        qint64 biggestSize = 0;
        QFileInfo fi(m_files.first());
        foreach (const QFileInfo &fiVob, fi.dir().entryInfoList(QStringList() << "VTS_*.VOB" << "vts_*.vob", QDir::Files, QDir::Name)) {
            QRegExp rx("VTS_([0-9]*)_[0-9]*.VOB");
            rx.setMinimal(true);
            rx.setCaseSensitivity(Qt::CaseInsensitive);
            if (rx.indexIn(fiVob.fileName()) != -1) {
                if (!sizes.contains(rx.cap(1)))
                    sizes.insert(rx.cap(1), 0);
                sizes[rx.cap(1)] += fiVob.size();
                if (sizes[rx.cap(1)] > biggestSize) {
                    biggestSize = sizes[rx.cap(1)];
                    biggest = rx.cap(1);
                }
            }
        }
        if (!biggest.isEmpty()) {
            QFileInfo fiNew(fi.absolutePath() + "/VTS_" + biggest + "_0.IFO");
            if (fiNew.isFile() && fiNew.exists())
                m_files = QStringList() << fiNew.absoluteFilePath();
        }
    }

    MediaInfo MI;
    MI.Option(QString("Info_Version").toStdWString(), QString("0.7.61;%1;%2").arg(QApplication::applicationName()).arg(QApplication::applicationVersion()).toStdWString());
    MI.Option(QString("Internet").toStdWString(), QString("no").toStdWString());

#ifdef Q_OS_WIN32
    MI.Open(m_files.first().toStdWString());
#else
    MI.Open(QString(m_files.first().toUtf8()).toStdWString());
#endif
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

    if (m_files.count() > 1) {
        foreach (const QString &file, m_files) {
            MediaInfo MI_duration;
            MI_duration.Option(QString("Info_Version").toStdWString(), QString("0.7.61;%1;%2").arg(QApplication::applicationName()).arg(QApplication::applicationVersion()).toStdWString());
            MI_duration.Option(QString("Internet").toStdWString(), QString("no").toStdWString());
        #ifdef Q_OS_WIN32
            MI_duration.Open(file.toStdWString());
        #else
            MI_duration.Open(QString(file.toUtf8()).toStdWString());
        #endif
            MI_duration.Option(QString("Complete").toStdWString(), QString("1").toStdWString());
            duration += qRound(QString::fromStdWString(MI.Get(Stream_General, 0, QString("Duration").toStdWString())).toFloat()/1000);
        }
    } else {
        duration = qRound(QString::fromStdWString(MI.Get(Stream_General, 0, QString("Duration").toStdWString())).toFloat()/1000);
    }

    setVideoDetail("durationinseconds", QString("%1").arg(duration));

    if (videoCount > 0) {
        aspectRatio = QString::fromStdWString(MI.Get(Stream_Video, 0, QString("DisplayAspectRatio").toStdWString())).toDouble();
        width = QString::fromStdWString(MI.Get(Stream_Video, 0, QString("Width").toStdWString())).toInt();
        height = QString::fromStdWString(MI.Get(Stream_Video, 0, QString("Height").toStdWString())).toInt();
        scanType = QString::fromStdWString(MI.Get(Stream_Video, 0, QString("ScanType").toStdWString()));

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
        QString audioCodec = audioFormat(QString::fromStdWString(MI.Get(Stream_Audio, i, QString("Codec").toStdWString())),
                                         QString::fromStdWString(MI.Get(Stream_Audio, i, QString("Format_Profile").toStdWString())));
        QString channels = QString::fromStdWString(MI.Get(Stream_Audio, i, QString("Channels").toStdWString()));
        QRegExp rx("^(\\d*)\\D*");
        if (rx.indexIn(QString("%1").arg(channels), 0) != -1)
            channels = rx.cap(1);
        else
            channels = "";
        setAudioDetail(i, "language", lang);
        setAudioDetail(i, "codec", audioCodec);
        setAudioDetail(i, "channels", channels);
    }

    for (int i=0 ; i<textCount ; ++i) {
        QString lang = QString::fromStdWString(MI.Get(Stream_Text, i, QString("Language/String3").toStdWString())).toLower();
        setSubtitleDetail(i, "language", lang);
    }
    MI.Close();
}

/**
 * @brief Modifies a video format name
 * @param format Original format, given by libstreaminfo
 * @param version Version, given by libstreaminfo
 * @return Modified format
 */
QString StreamDetails::videoFormat(QString format, QString version)
{
    format = format.toLower();
    if (!format.isEmpty() && format == "mpeg video")
        format = (version.toLower() == "version 2") ? "mpeg2" : "mpeg";
    if (Settings::instance()->advanced()->videoCodecMappings().contains(format))
        return Settings::instance()->advanced()->videoCodecMappings().value(format);
    return format;
}

/**
 * @brief Returns a modified audio format
 * @param format Original format, given by libstreaminfo
 * @return Modified format
 */
QString StreamDetails::audioFormat(const QString &codec, const QString &profile)
{
    QString xbmcFormat;
    if (codec == "DTS-HD" && profile == "MA / Core")
        xbmcFormat = "dtshd_ma";
    else if (codec == "DTS-HD" && profile == "HRA / Core")
        xbmcFormat = "dtshd_hra";
    else if (codec == "AC3")
        xbmcFormat = "ac3";
    else if (codec == "AC3+" || codec == "E-AC-3")
        xbmcFormat = "eac3";
    else if (codec == "TrueHD / AC3")
        xbmcFormat = "truehd";
    else if (codec == "FLAC")
        xbmcFormat = "flac";
    else if (codec == "MPA1L3")
        xbmcFormat = "mp3";
    else
        xbmcFormat = codec;

    if (Settings::instance()->advanced()->audioCodecMappings().contains(xbmcFormat))
        return Settings::instance()->advanced()->audioCodecMappings().value(xbmcFormat);
    return xbmcFormat;
}

/**
 * @brief Sets a video detail
 * @param key The key (aspect, width, height...)
 * @param value The value
 */
void StreamDetails::setVideoDetail(QString key, QString value)
{
    m_videoDetails.insert(key, value);
}

/**
 * @brief Sets a audio detail
 * @param streamNumber Number of the stream
 * @param key Key (language, codec or channels)
 * @param value Value
 */
void StreamDetails::setAudioDetail(int streamNumber, QString key, QString value)
{
    if (streamNumber >= m_audioDetails.count())
        m_audioDetails.insert(streamNumber, QMap<QString, QString>());
    if (streamNumber >= m_audioDetails.count())
        return;
    m_audioDetails[streamNumber].insert(key, value);
}

/**
 * @brief Sets a subtitle detail
 * @param streamNumber Number of the stream
 * @param key Key (language)
 * @param value Language
 */
void StreamDetails::setSubtitleDetail(int streamNumber, QString key, QString value)
{
    if (streamNumber >= m_subtitles.count())
        m_subtitles.insert(streamNumber, QMap<QString, QString>());
    if (streamNumber >= m_subtitles.count())
        return;
    m_subtitles[streamNumber].insert(key, value);
}

/**
 * @brief Access video details
 * @return
 */
QMap<QString, QString> StreamDetails::videoDetails()
{
    return m_videoDetails;
}

/**
 * @brief Access audio details
 * @return
 */
QList<QMap<QString, QString> > StreamDetails::audioDetails()
{
    return m_audioDetails;
}

/**
 * @brief Access subtitles
 * @return
 */
QList<QMap<QString, QString> > StreamDetails::subtitleDetails()
{
    return m_subtitles;
}
