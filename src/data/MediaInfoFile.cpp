#include "data/MediaInfoFile.h"

#include "globals/Helper.h"
#include "settings/Settings.h"

#include "MediaInfoDLL/MediaInfoDLL.h"

#include <ZenLib/Ztring.h>
#include <ZenLib/ZtringListList.h>

#include <QDebug>

#ifdef Q_OS_WIN
#define QString2MI(_DATA) QString(_DATA).toStdWString()
#define MI2QString(_DATA) QString::fromStdWString(_DATA)
#else
#define QString2MI(_DATA) QString{_DATA}.toUtf8().data()
#define MI2QString(_DATA) QString((_DATA).c_str())
#endif

MediaInfoFile::MediaInfoFile(const QString& filepath) : m_mediaInfo{std::make_unique<MediaInfoDLL::MediaInfo>()}
{
    // VERSION;APP_NAME;APP_VERSION"
    m_mediaInfo->Option(__T("Info_Version"), __T("19.09;MediaElch;2.6"));
    m_mediaInfo->Option(__T("Internet"), __T("no"));
    m_mediaInfo->Option(__T("Complete"), __T("1"));
    m_mediaInfo->Open(QString2MI(filepath));
    if (!m_mediaInfo->IsReady()) {
        qCritical() << "[MediaInfo] Not able to load libmediainfo!";
    }
}

MediaInfoFile::~MediaInfoFile()
{
    m_mediaInfo->Close();
}

int MediaInfoFile::subtitleCount() const
{
    return getGeneral(0, "TextCount").toInt();
}

int MediaInfoFile::videoStreamCount() const
{
    return getGeneral(0, "VideoCount").toInt();
}

int MediaInfoFile::audioStreamCount() const
{
    return getGeneral(0, "AudioCount").toInt();
}

std::chrono::milliseconds MediaInfoFile::duration(int streamIndex) const
{
    return std::chrono::milliseconds(getGeneral(streamIndex, "Duration").toLongLong());
}

std::size_t MediaInfoFile::videoWidth(int streamIndex) const
{
    long long width = getVideo(streamIndex, "Width").toLongLong();
    return width < 0 ? 0 : width;
}

std::size_t MediaInfoFile::videoHeight(int streamIndex) const
{
    long long height = getVideo(streamIndex, "Height").toLongLong();
    return height < 0 ? 0 : height;
}

double MediaInfoFile::aspectRatio(int streamIndex) const
{
    return getVideo(streamIndex, "DisplayAspectRatio").toDouble();
}

QString MediaInfoFile::codec(int streamIndex) const
{
    QString codec = getVideo(streamIndex, "Format");
    if (codec.isEmpty()) {
        codec = getVideo(streamIndex, "CodecID");
    }
    return codec;
}

QString MediaInfoFile::mpegVersion(int streamIndex) const
{
    return getVideo(streamIndex, "Format_Version");
}

QString MediaInfoFile::scanType(int streamIndex) const
{
    if (getVideo(streamIndex, "CodecID") == "V_MPEGH/ISO/HEVC") {
        return "progressive";
    }
    QString scanType = getVideo(streamIndex, "ScanType");
    if (scanType == "MBAFF") {
        return "interlaced";
    }
    return scanType.toLower();
}

QString MediaInfoFile::stereoFormat(int streamIndex) const
{
    QString multiView = getVideo(streamIndex, "MultiView_Layout").toLower();
    if (helper::stereoModes().values().contains(multiView)) {
        return helper::stereoModes().key(multiView);
    }
    return "";
}

QString MediaInfoFile::format(int streamIndex) const
{
    return parseVideoFormat(codec(streamIndex), mpegVersion(streamIndex));
}

QString MediaInfoFile::audioLanguage(int streamIndex) const
{
    QString lang = getAudio(streamIndex, "Language/String3");
    if (lang.isEmpty()) {
        lang = getAudio(streamIndex, "Language/String2");
    }
    if (lang.isEmpty()) {
        lang = getAudio(streamIndex, "Language/String1");
    }
    if (lang.isEmpty()) {
        lang = getAudio(streamIndex, "Language/String");
    }
    return lang;
}

QString MediaInfoFile::audioCodec(int streamIndex) const
{
    QString format = getAudio(streamIndex, "Format");
    QString codecId = getAudio(streamIndex, "CodecID");
    return parseAudioFormat(format, codecId);
}

QString MediaInfoFile::audioChannels(int streamIndex) const
{
    QString channels = getAudio(streamIndex, "Channel(s)");
    QString channelsOriginal = getAudio(streamIndex, "Channel(s)_Original");
    if (!channelsOriginal.isEmpty()) {
        channels = channelsOriginal;
    }
    QRegExp rx(R"(^\D*(\d*)\D*)");
    if (rx.indexIn(channels, 0) != -1) {
        channels = rx.cap(1);
    } else {
        channels = "";
    }
    return channels;
}

QString MediaInfoFile::subtitleLang(int streamIndex) const
{
    QString lang = getText(streamIndex, "Language/String3");
    if (lang.isEmpty()) {
        lang = getText(streamIndex, "Language/String2");
    }
    if (lang.isEmpty()) {
        lang = getText(streamIndex, "Language/String1");
    }
    if (lang.isEmpty()) {
        lang = getText(streamIndex, "Language/String");
    }
    return lang;
}

/// Returns a modified audio format
/// @param codec Original codec format, given by libmediainfo
/// @return Modified format
QString MediaInfoFile::parseAudioFormat(const QString& codec, const QString& profile) const
{
    QString xbmcFormat;
    if (codec == "DTS-HD" && profile == "MA / Core") {
        xbmcFormat = "dtshd_ma";
    } else if (codec == "DTS-HD" && profile == "HRA / Core") {
        xbmcFormat = "dtshd_hra";
    } else if (codec == "DTS-HD" && profile == "X / MA / Core") {
        xbmcFormat = "dtshd_x";
    } else if (codec == "AC3" || codec == "AC-3") {
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
    } else if (codec == "AAC LC" || codec == "AAC") {
        xbmcFormat = "aac";
    } else {
        xbmcFormat = codec.toLower();
    }

    if (Settings::instance()->advanced()->audioCodecMappings().contains(xbmcFormat)) {
        return Settings::instance()->advanced()->audioCodecMappings().value(xbmcFormat);
    }
    return xbmcFormat;
}


/**
 * @brief Modifies a video format name
 * @param format Original format, given by libmediainfo
 * @param version Version, given by libmediainfo
 * @return Modified format
 */
QString MediaInfoFile::parseVideoFormat(QString format, QString version) const
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

QString MediaInfoFile::getGeneral(int streamIndex, const char* parameter) const
{
    return MI2QString(m_mediaInfo->Get(MediaInfoDLL::Stream_General, streamIndex, QString2MI(parameter)));
}

QString MediaInfoFile::getAudio(int streamIndex, const char* parameter) const
{
    return MI2QString(m_mediaInfo->Get(MediaInfoDLL::Stream_Audio, streamIndex, QString2MI(parameter)));
}

QString MediaInfoFile::getVideo(int streamIndex, const char* parameter) const
{
    return MI2QString(m_mediaInfo->Get(MediaInfoDLL::Stream_Video, streamIndex, QString2MI(parameter)));
}

QString MediaInfoFile::getText(int streamIndex, const char* parameter) const
{
    return MI2QString(m_mediaInfo->Get(MediaInfoDLL::Stream_Text, streamIndex, QString2MI(parameter)));
}
