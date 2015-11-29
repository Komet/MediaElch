#include "StreamDetails.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include "globals/Helper.h"
#include "settings/Settings.h"

#include "MediaInfoDLL/MediaInfoDLL.h"
using namespace MediaInfoDLL;

#include <ZenLib/Ztring.h>
#include <ZenLib/ZtringListList.h>
using namespace ZenLib;

#ifdef Q_OS_WIN
    #define QString2MI(_DATA) QString(_DATA).toStdWString()
    #define MI2QString(_DATA) QString::fromStdWString(_DATA)
#else
    #define QString2MI(_DATA) QString(_DATA).toUtf8().data()
    #define MI2QString(_DATA) QString(_DATA.c_str())
#endif

/**
 * @brief StreamDetails::StreamDetails
 * @param parent
 * @param file
 */
StreamDetails::StreamDetails(QObject *parent, QStringList files) :
    QObject(parent)
{
    m_files = files;
    m_hdAudioCodecs << "dtshd_ma" << "dtshd_hra" << "truehd";
    m_normalAudioCodecs << "DTS" << "dts" << "ac3" << "eac3" << "flac";
    m_sdAudioCodecs << "mp3";
}

/**
 * @brief Clears all information
 */
void StreamDetails::clear()
{
    m_videoDetails.clear();
    m_audioDetails.clear();
    m_subtitles.clear();
    m_availableChannels.clear();
    m_availableQualities.clear();
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

    loadWithLibrary();
}

void StreamDetails::loadWithLibrary()
{
    MediaInfo MI;
    MI.Option(__T("Info_Version"), __T("0.7.70;MediaElch;2"));
    MI.Option(__T("Internet"), __T("no"));
    MI.Option(__T("Complete"), __T("1"));

    QString fileName = m_files.first();
    if (m_files.count() == 1 && m_files.first().endsWith("index.bdmv")) {
        QFileInfo fi(fileName);
        QDir dir(fi.absolutePath() + "/STREAM");
        QStringList files = dir.entryList(QStringList() << "*.m2ts", QDir::NoDotAndDotDot | QDir::Files, QDir::Name);
        if (!files.isEmpty())
            fileName = dir.absolutePath() + "/" + files.first();
    }

    MI.Open(QString2MI(fileName));

    int duration = 0;
    double aspectRatio;
    int width = 0;
    int height = 0;
    QString scanType;
    QString videoCodec;

    int videoCount = MI2QString(MI.Get(Stream_General, 0, QString2MI("VideoCount"))).toInt();
    int audioCount = MI2QString(MI.Get(Stream_General, 0, QString2MI("AudioCount"))).toInt();
    int textCount = MI2QString(MI.Get(Stream_General, 0, QString2MI("TextCount"))).toInt();

    if (m_files.count() > 1) {
        foreach (const QString &file, m_files) {
            MediaInfo MI_duration;
            MI_duration.Option(__T("Info_Version"), __T("0.7.70;MediaElch;2"));
            MI_duration.Option(__T("Internet"), __T("no"));
            MI_duration.Option(__T("Complete"), __T("1"));
            MI_duration.Open(QString2MI(file));
            duration += qRound(MI2QString(MI_duration.Get(Stream_General, 0, QString2MI("Duration"))).toFloat()/1000);
            MI_duration.Close();
        }
    } else {
        duration += qRound(MI2QString(MI.Get(Stream_General, 0, QString2MI("Duration"))).toFloat()/1000);
    }

    setVideoDetail("durationinseconds", QString("%1").arg(duration));

    if (videoCount > 0) {
        aspectRatio = MI2QString(MI.Get(Stream_Video, 0, QString2MI("DisplayAspectRatio"))).toDouble();
        width = MI2QString(MI.Get(Stream_Video, 0, QString2MI("Width"))).toInt();
        height = MI2QString(MI.Get(Stream_Video, 0, QString2MI("Height"))).toInt();
        scanType = MI2QString(MI.Get(Stream_Video, 0, QString2MI("ScanType")));

        QString codec = MI2QString(MI.Get(Stream_Video, 0, QString2MI("CodecID/Hint")));
        QString version;
        if (codec.isEmpty()) {
            codec = MI2QString(MI.Get(Stream_Video, 0, QString2MI("CodecID")));
            if (codec.isEmpty()) {
                codec = MI2QString(MI.Get(Stream_Video, 0, QString2MI("Format")));
                version = MI2QString(MI.Get(Stream_Video, 0, QString2MI("Format_Version")));
            }
        }
        videoCodec = videoFormat(codec, version);

        QString multiView = MI2QString(MI.Get(Stream_Video, 0, QString2MI("MultiView_Layout")));

        setVideoDetail("codec", videoCodec);
        setVideoDetail("aspect", QString("%1").arg(aspectRatio));
        setVideoDetail("width", QString("%1").arg(width));
        setVideoDetail("height", QString("%1").arg(height));
        setVideoDetail("scantype", scanType.toLower());
        setVideoDetail("stereomode", stereoFormat(multiView));
    }

    for (int i=0 ; i<audioCount ; ++i) {
        QString lang = MI2QString(MI.Get(Stream_Audio, i, QString2MI("Language/String3")));
        QString audioCodec = audioFormat(MI2QString(MI.Get(Stream_Audio, i, QString2MI("Codec"))),
                                         MI2QString(MI.Get(Stream_Audio, i, QString2MI("Format_Profile"))));
        QString channels = MI2QString(MI.Get(Stream_Audio, i, QString2MI("Channel(s)")));
        if (!MI2QString(MI.Get(Stream_Audio, i, QString2MI("Channel(s)_Original"))).isEmpty())
            channels = MI2QString(MI.Get(Stream_Audio, i, QString2MI("Channel(s)_Original")));
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
        QString lang = MI2QString(MI.Get(Stream_Text, i, QString2MI("Languange/String3")));
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
    return format.toLower();
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
    return xbmcFormat.toLower();
}

QString StreamDetails::stereoFormat(const QString &format)
{
    if (Helper::instance()->stereoModes().contains(format.toLower()))
        return Helper::instance()->stereoModes().value(format.toLower());
    return "";
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

    if (key == "channels" && !m_availableChannels.contains(value.toInt()))
        m_availableChannels.append(value.toInt());

    if (key == "codec" && m_hdAudioCodecs.contains(value) && !m_availableQualities.contains("hd"))
        m_availableQualities.append("hd");
    else if (key == "codec" && m_normalAudioCodecs.contains(value) && !m_availableQualities.contains("normal"))
        m_availableQualities.append("normal");
    else if (key == "codec" && m_sdAudioCodecs.contains(value) && !m_availableQualities.contains("sd"))
        m_availableQualities.append("sd");
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

bool StreamDetails::hasAudioChannels(int channels)
{
    return m_availableChannels.contains(channels);
}

bool StreamDetails::hasAudioQuality(QString quality)
{
    return m_availableQualities.contains(quality);
}

int StreamDetails::audioChannels()
{
    int channels = 0;
    foreach (int c, m_availableChannels) {
        if (c > channels)
            channels = c;
    }
    return channels;
}

QString StreamDetails::audioCodec()
{
    QString hdCodec;
    QString normalCodec;
    QString sdCodec;
    for (int i=0, n=m_audioDetails.count() ; i<n ; ++i) {
        QString codec = m_audioDetails.at(i).value("codec");
        if (m_hdAudioCodecs.contains(codec))
            hdCodec = codec;
        if (m_normalAudioCodecs.contains(codec))
            normalCodec = codec;
        if (m_sdAudioCodecs.contains(codec))
            sdCodec = codec;
    }

    if (!hdCodec.isEmpty())
        return hdCodec;
    if (!normalCodec.isEmpty())
        return normalCodec;
    if (!sdCodec.isEmpty())
        return sdCodec;
    return "";
}

QString StreamDetails::videoCodec()
{
    return m_videoDetails.value("codec");
}
