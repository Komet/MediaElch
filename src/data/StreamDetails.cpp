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
#define QString2MI(_DATA) QString{_DATA}.toUtf8().data()
#define MI2QString(_DATA) QString((_DATA).c_str())
#endif

/**
 * @brief StreamDetails::StreamDetails
 */
StreamDetails::StreamDetails(QObject* parent, QStringList files) :
    QObject(parent),
    m_files(std::move(files)),
    m_hdAudioCodecs{"dtshd_ma", "dtshd_hra", "truehd"},
    m_normalAudioCodecs{"DTS", "dts", "ac3", "eac3", "flac"},
    m_sdAudioCodecs{"mp3"}
{
}

QString StreamDetails::detailToString(VideoDetails details)
{
    switch (details) {
    case VideoDetails::Codec: return "codec";
    case VideoDetails::Aspect: return "aspect";
    case VideoDetails::Width: return "width";
    case VideoDetails::Height: return "height";
    case VideoDetails::DurationInSeconds: return "durationinseconds";
    case VideoDetails::ScanType: return "scantype";
    case VideoDetails::StereoMode: return "stereomode";
    }
    qWarning() << "Undefined video detail: no string representation";
    return "undefined";
}

QString StreamDetails::detailToString(AudioDetails details)
{
    switch (details) {
    case StreamDetails::AudioDetails::Codec: return "codec";
    case StreamDetails::AudioDetails::Language: return "language";
    case StreamDetails::AudioDetails::Channels: return "channels";
    }
    qWarning() << "Undefined audio detail: no string representation";
    return "undefined";
}

QString StreamDetails::detailToString(SubtitleDetails details)
{
    switch (details) {
    case StreamDetails::SubtitleDetails::Language: return "language";
    }
    qWarning() << "Undefined subtitle detail: no string representation";
    return "undefined";
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
    if (m_files.isEmpty()) {
        return;
    }

    if (m_files.first().endsWith(".iso", Qt::CaseInsensitive)
        || m_files.first().endsWith(".img", Qt::CaseInsensitive)) {
        return;
    }

    // If it's a DVD structure, compute the biggest part (main movie) and use this IFO file
    if (m_files.first().endsWith("VIDEO_TS.IFO")) {
        QMap<QString, qint64> sizes;
        QString biggest;
        qint64 biggestSize = 0;
        QFileInfo fi(m_files.first());
        for (const QFileInfo& fiVob :
            fi.dir().entryInfoList(QStringList{"VTS_*.VOB", "vts_*.vob"}, QDir::Files, QDir::Name)) {
            QRegExp rx("VTS_([0-9]*)_[0-9]*.VOB");
            rx.setMinimal(true);
            rx.setCaseSensitivity(Qt::CaseInsensitive);
            if (rx.indexIn(fiVob.fileName()) != -1) {
                if (!sizes.contains(rx.cap(1))) {
                    sizes.insert(rx.cap(1), 0);
                }
                sizes[rx.cap(1)] += fiVob.size();
                if (sizes[rx.cap(1)] > biggestSize) {
                    biggestSize = sizes[rx.cap(1)];
                    biggest = rx.cap(1);
                }
            }
        }
        if (!biggest.isEmpty()) {
            QFileInfo fiNew(fi.absolutePath() + "/VTS_" + biggest + "_0.IFO");
            if (fiNew.isFile() && fiNew.exists()) {
                m_files = QStringList() << fiNew.absoluteFilePath();
            }
        }
    }

    loadWithLibrary();
}

void StreamDetails::loadWithLibrary()
{
    MediaInfo mi;
    // VERSION;APP_NAME;APP_VERSION")
    mi.Option(__T("Info_Version"), __T("17.12;MediaElch;2.6"));
    mi.Option(__T("Internet"), __T("no"));
    mi.Option(__T("Complete"), __T("1"));

    QString fileName = m_files.first();
    if (m_files.count() == 1 && m_files.first().endsWith("index.bdmv")) {
        QFileInfo fi(fileName);
        QDir dir(fi.absolutePath() + "/STREAM");
        QStringList files = dir.entryList(QStringList() << "*.m2ts", QDir::NoDotAndDotDot | QDir::Files, QDir::Name);
        if (!files.isEmpty()) {
            fileName = dir.absolutePath() + "/" + files.first();
        }
    }

    mi.Open(QString2MI(fileName));

    int duration = 0;
    QString scanType;
    QString videoCodec;

    int videoCount = MI2QString(mi.Get(Stream_General, 0, QString2MI("VideoCount"))).toInt();
    int audioCount = MI2QString(mi.Get(Stream_General, 0, QString2MI("AudioCount"))).toInt();
    int textCount = MI2QString(mi.Get(Stream_General, 0, QString2MI("TextCount"))).toInt();

    if (m_files.count() > 1) {
        for (const QString& file : m_files) {
            MediaInfo miDuration;
            miDuration.Option(__T("Info_Version"), __T("17.12;MediaElch;2.6"));
            miDuration.Option(__T("Internet"), __T("no"));
            miDuration.Option(__T("Complete"), __T("1"));
            miDuration.Open(QString2MI(file));
            duration += qRound(MI2QString(miDuration.Get(Stream_General, 0, QString2MI("Duration"))).toFloat() / 1000);
            miDuration.Close();
        }
    } else {
        duration += qRound(MI2QString(mi.Get(Stream_General, 0, QString2MI("Duration"))).toFloat() / 1000);
    }

    setVideoDetail(StreamDetails::VideoDetails::DurationInSeconds, QString::number(duration));

    if (videoCount > 0) {
        double aspectRatio = MI2QString(mi.Get(Stream_Video, 0, QString2MI("DisplayAspectRatio"))).toDouble();
        int width = MI2QString(mi.Get(Stream_Video, 0, QString2MI("Width"))).toInt();
        int height = MI2QString(mi.Get(Stream_Video, 0, QString2MI("Height"))).toInt();

        QString codec = MI2QString(mi.Get(Stream_Video, 0, QString2MI("Format")));
        if (codec.isEmpty()) {
            codec = MI2QString(mi.Get(Stream_Video, 0, QString2MI("CodecID")));
        }

        QString version = MI2QString(mi.Get(Stream_Video, 0, QString2MI("Format_Version")));

        videoCodec = videoFormat(codec, version);

        if (MI2QString(mi.Get(Stream_Video, 0, QString2MI("CodecID"))) == "V_MPEGH/ISO/HEVC") {
            scanType = "progressive";
        } else {
            scanType = MI2QString(mi.Get(Stream_Video, 0, QString2MI("ScanType")));
            if (scanType == "MBAFF") {
                scanType = "interlaced";
            }
        }

        QString multiView = MI2QString(mi.Get(Stream_Video, 0, QString2MI("MultiView_Layout")));

        setVideoDetail(VideoDetails::Codec, videoCodec);
        setVideoDetail(VideoDetails::Aspect, QString("%1").arg(aspectRatio));
        setVideoDetail(VideoDetails::Width, QString("%1").arg(width));
        setVideoDetail(VideoDetails::Height, QString("%1").arg(height));
        setVideoDetail(VideoDetails::ScanType, scanType.toLower());
        setVideoDetail(VideoDetails::StereoMode, stereoFormat(multiView));
    }

    for (int i = 0; i < audioCount; ++i) {
        QString lang = MI2QString(mi.Get(Stream_Audio, i, QString2MI("Language/String3")));
        if (lang.isEmpty()) {
            lang = MI2QString(mi.Get(Stream_Audio, i, QString2MI("Language/String2")));
        }
        if (lang.isEmpty()) {
            lang = MI2QString(mi.Get(Stream_Audio, i, QString2MI("Language/String1")));
        }
        if (lang.isEmpty()) {
            lang = MI2QString(mi.Get(Stream_Audio, i, QString2MI("Language/String")));
        }
        QString audioCodec = audioFormat(MI2QString(mi.Get(Stream_Audio, i, QString2MI("Codec"))),
            MI2QString(mi.Get(Stream_Audio, i, QString2MI("Format_Profile"))));
        QString channels = MI2QString(mi.Get(Stream_Audio, i, QString2MI("Channel(s)")));
        if (!MI2QString(mi.Get(Stream_Audio, i, QString2MI("Channel(s)_Original"))).isEmpty()) {
            channels = MI2QString(mi.Get(Stream_Audio, i, QString2MI("Channel(s)_Original")));
        }
        QRegExp rx(R"(^\D*(\d*)\D*)");
        if (rx.indexIn(QString("%1").arg(channels), 0) != -1) {
            channels = rx.cap(1);
        } else {
            channels = "";
        }
        setAudioDetail(i, AudioDetails::Language, lang);
        setAudioDetail(i, AudioDetails::Codec, audioCodec);
        setAudioDetail(i, AudioDetails::Channels, channels);
    }

    for (int i = 0; i < textCount; ++i) {
        QString lang = MI2QString(mi.Get(Stream_Text, i, QString2MI("Language/String3")));
        if (lang.isEmpty()) {
            lang = MI2QString(mi.Get(Stream_Text, i, QString2MI("Language/String2")));
        }
        if (lang.isEmpty()) {
            lang = MI2QString(mi.Get(Stream_Text, i, QString2MI("Language/String1")));
        }
        if (lang.isEmpty()) {
            lang = MI2QString(mi.Get(Stream_Text, i, QString2MI("Language/String")));
        }
        setSubtitleDetail(i, StreamDetails::SubtitleDetails::Language, lang);
    }

    mi.Close();
}

/**
 * @brief Modifies a video format name
 * @param format Original format, given by libstreaminfo
 * @param version Version, given by libstreaminfo
 * @return Modified format
 */
QString StreamDetails::videoFormat(QString format, QString version) const
{
    format = format.toLower();
    if (!format.isEmpty() && format == "mpeg video") {
        format = (version.toLower() == "version 2") ? "mpeg2" : "mpeg";
    }
    if (Settings::instance()->advanced()->videoCodecMappings().contains(format)) {
        return Settings::instance()->advanced()->videoCodecMappings().value(format);
    }
    return format.toLower();
}

/**
 * @brief Returns a modified audio format
 * @param codec Original codec format, given by libstreaminfo
 * @return Modified format
 */
QString StreamDetails::audioFormat(const QString& codec, const QString& profile) const
{
    QString xbmcFormat;
    if (codec == "DTS-HD" && profile == "MA / Core") {
        xbmcFormat = "dtshd_ma";
    } else if (codec == "DTS-HD" && profile == "HRA / Core") {
        xbmcFormat = "dtshd_hra";
    } else if (codec == "DTS-HD" && profile == "X / MA / Core") {
        xbmcFormat = "dtshd_x";
    } else if (codec == "AC3") {
        xbmcFormat = "ac3";
    } else if (codec == "AC3+" || codec == "E-AC-3") {
        xbmcFormat = "eac3";
    } else if (codec == "TrueHD / AC3") {
        xbmcFormat = "truehd";
    } else if (codec == "TrueHD" && profile == "TrueHD+Atmos / TrueHD") {
        xbmcFormat = "atmos";
    } else if (codec == "FLAC") {
        xbmcFormat = "flac";
    } else if (codec == "MPA1L3") {
        xbmcFormat = "mp3";
    } else {
        xbmcFormat = codec.toLower();
    }

    if (Settings::instance()->advanced()->audioCodecMappings().contains(xbmcFormat)) {
        return Settings::instance()->advanced()->audioCodecMappings().value(xbmcFormat);
    }
    return xbmcFormat;
}

QString StreamDetails::stereoFormat(const QString& format) const
{
    if (helper::stereoModes().values().contains(format.toLower())) {
        return helper::stereoModes().key(format.toLower());
    }
    return "";
}

/**
 * @brief Sets a video detail
 * @param key The key (aspect, width, height...)
 * @param value The value
 */
void StreamDetails::setVideoDetail(VideoDetails key, QString value)
{
    m_videoDetails.insert(key, value);
}

/**
 * @brief Sets a audio detail
 * @param streamNumber Number of the stream
 * @param key Key (language, codec or channels)
 * @param value Value
 */
void StreamDetails::setAudioDetail(int streamNumber, AudioDetails key, QString value)
{
    if (streamNumber >= m_audioDetails.count()) {
        m_audioDetails.resize(streamNumber);
        m_audioDetails.insert(streamNumber, QMap<AudioDetails, QString>{{key, value}});
        return;
    }
    m_audioDetails[streamNumber].insert(key, value);

    if (key == AudioDetails::Channels && !m_availableChannels.contains(value.toInt())) {
        m_availableChannels.append(value.toInt());
    }
    if (key == AudioDetails::Codec) {
        if (m_hdAudioCodecs.contains(value) && !m_availableQualities.contains("hd")) {
            m_availableQualities.append("hd");
        } else if (m_normalAudioCodecs.contains(value) && !m_availableQualities.contains("normal")) {
            m_availableQualities.append("normal");
        } else if (m_sdAudioCodecs.contains(value) && !m_availableQualities.contains("sd")) {
            m_availableQualities.append("sd");
        }
    }
}

/**
 * @brief Sets a subtitle detail
 * @param streamNumber Number of the stream
 * @param key Key (language)
 * @param value Language
 */
void StreamDetails::setSubtitleDetail(int streamNumber, SubtitleDetails key, QString value)
{
    if (streamNumber >= m_subtitles.count()) {
        m_subtitles.resize(streamNumber + 1);
        m_subtitles.insert(streamNumber, QMap<SubtitleDetails, QString>{{key, value}});
        return;
    }
    m_subtitles[streamNumber].insert(key, value);
}

/**
 * @brief Access video details
 */
QMap<StreamDetails::VideoDetails, QString> StreamDetails::videoDetails() const
{
    return m_videoDetails;
}

/**
 * @brief Access audio details
 */
QVector<QMap<StreamDetails::AudioDetails, QString>> StreamDetails::audioDetails() const
{
    return m_audioDetails;
}

/**
 * @brief Access subtitles
 */
QVector<QMap<StreamDetails::SubtitleDetails, QString>> StreamDetails::subtitleDetails() const
{
    return m_subtitles;
}

bool StreamDetails::hasAudioChannels(int channels) const
{
    return m_availableChannels.contains(channels);
}

bool StreamDetails::hasAudioQuality(QString quality) const
{
    return m_availableQualities.contains(quality);
}

int StreamDetails::audioChannels() const
{
    int channels = 0;
    for (int c : m_availableChannels) {
        if (c > channels) {
            channels = c;
        }
    }
    return channels;
}

QString StreamDetails::audioCodec() const
{
    QString hdCodec;
    QString normalCodec;
    QString sdCodec;
    for (int i = 0, n = m_audioDetails.count(); i < n; ++i) {
        QString codec = m_audioDetails.at(i).value(AudioDetails::Codec);
        if (m_hdAudioCodecs.contains(codec)) {
            hdCodec = codec;
        }
        if (m_normalAudioCodecs.contains(codec)) {
            normalCodec = codec;
        }
        if (m_sdAudioCodecs.contains(codec)) {
            sdCodec = codec;
        }
    }

    if (!hdCodec.isEmpty()) {
        return hdCodec;
    }
    if (!normalCodec.isEmpty()) {
        return normalCodec;
    }
    if (!sdCodec.isEmpty()) {
        return sdCodec;
    }
    return "";
}

QString StreamDetails::videoCodec() const
{
    return m_videoDetails.value(VideoDetails::Codec);
}
