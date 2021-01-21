#include "data/MediaInfoFile.h"

#include "globals/Helper.h"
#include "settings/Settings.h"

#include "MediaInfoDLL/MediaInfoDLL.h"

#include <ZenLib/Ztring.h>
#include <ZenLib/ZtringListList.h>

#include <QDebug>
#include <QRegularExpression>
#include <QStringList>

#ifdef Q_OS_WIN
#    define QString2MI(_DATA) QString(_DATA).toStdWString()
#    define MI2QString(_DATA) QString::fromStdWString(_DATA)
#else
#    define QString2MI(_DATA) QString{_DATA}.toUtf8().data()
#    define MI2QString(_DATA) QString((_DATA).c_str())
#endif

MediaInfoFile::MediaInfoFile(const QString& filepath) : m_mediaInfo{std::make_unique<MediaInfoDLL::MediaInfo>()}
{
    // VERSION;APP_NAME;APP_VERSION"
    m_mediaInfo->Option(__T("Info_Version"), __T("20.03;MediaElch;2.6"));
    m_mediaInfo->Option(__T("Internet"), __T("no"));
    m_mediaInfo->Option(__T("Complete"), __T("1"));
    m_mediaInfo->Open(QString2MI(filepath));
    if (!m_mediaInfo->IsReady()) {
        qCritical() << "[MediaInfo] Unable to load libmediainfo!";
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
    const QString durationInMilliseconds = getGeneral(streamIndex, "Duration");
    // We are only interested in anything lower than milliseconds, so the cast is fine.
    return std::chrono::milliseconds(static_cast<qint64>(durationInMilliseconds.toDouble()));
}

std::size_t MediaInfoFile::videoWidth(int streamIndex) const
{
    // Width should be an integer but just to be safe, use a floating point number first.
    const double widthDouble = getVideo(streamIndex, "Width").toDouble();
    const qint64 width = static_cast<qint64>(widthDouble);
    return width < 0 ? 0 : width;
}

std::size_t MediaInfoFile::videoHeight(int streamIndex) const
{
    // Height should be an integer but just to be safe, use a floating point number first.
    const double heightDouble = getVideo(streamIndex, "Height").toDouble();
    const qint64 height = static_cast<qint64>(heightDouble);
    return height < 0 ? 0 : height;
}

double MediaInfoFile::aspectRatio(int streamIndex) const
{
    QString str = getVideo(streamIndex, "DisplayAspectRatio");
    return str.toDouble();
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
    /// Audio codec for Kodi
    QString audioCodec;

    // Workaround for DTS & TrueHD variant detection
    // Search for well known strings in defined keys (there are changes between different MI versions!)
    // Idea from TinyMediaManager:
    // https://gitlab.com/tinyMediaManager/tinyMediaManager/blob/3a3f9743ff52cd68e1e50764b65531cc139ef152/src/main/java/org/tinymediamanager/core/MediaFileHelper.java#L1251-1264
    QStringList searchCodecs = getAudio(streamIndex,
        QStringList{
            "Format",                  // e.g. MLP FBA
            "Format_Profile",          //
            "Format_Commercial",       // e.g. "Dolby TrueHD"
            "Format_Commercial_IfAny", // e.g. "Dolby TrueHD"
            "CodecID",                 // e.g. A_TRUEHD => A_ == "Audio"
            "Codec"                    // deprecated in newer MediaInfo lib versions
        });

    if (helper::containsIgnoreCase(searchCodecs, "TrueHD")) {
        audioCodec = "truehd";
    } else if (helper::containsIgnoreCase(searchCodecs, "Atmos")) {
        audioCodec = "atmos";
    } else if (helper::containsIgnoreCase(searchCodecs, "DTS")) {
        audioCodec = "dts";
    } else {
        // Default Case: "Format" should be used
        audioCodec = getAudio(streamIndex, "Format").toLower();
    }

    // https://github.com/Radarr/Radarr/blob/420e5fd730dbc9df339390674ce256e739b143cc/src/NzbDrone.Core/MediaFiles/MediaInfo/MediaInfoFormatter.cs#L73
    QString addFeature = getAudio(streamIndex, "Format_AdditionalFeatures");
    if (!addFeature.isEmpty()) {
        if (audioCodec == "dts") {
            if (addFeature.startsWith("XLL")) {
                if (addFeature.endsWith("X")) {
                    audioCodec = "dtshd_x";
                } else {
                    audioCodec = "dtshd_ma";
                }
            } else if (addFeature == "ES") {
                audioCodec = "dtshd_es";
            } else if (addFeature == "XBR") {
                audioCodec = "dtshd_hra";
            }
            // "normal" dts
        }
        if (audioCodec == "truehd") {
            if (addFeature.toLower() == "16-ch") {
                audioCodec = "atmos";
            }
        }
    }

    // Logic similiar to TMM
    // old <= 18.05 style
    QString audioProfile = getAudio(streamIndex, "Format_Profile");
    if (!audioProfile.isEmpty()) {
        if (audioCodec == "dts") {
            if (audioProfile.contains("ES")) {
                audioCodec = "dtshd_es";

            } else if (audioProfile.contains("HRA")) { // Profile: "HRA / Core"
                audioCodec = "dtshd_hra";

            } else if (audioProfile.contains("MA")) {
                if (audioProfile.contains("X")) { // Profile: "X / MA / Core"
                    audioCodec = "dtshd_x";
                } else {
                    audioCodec = "dtshd_ma"; // Profile: "MA / Core"
                }
            }
        }
        if (audioCodec == "truehd") {
            if (audioProfile.contains("Atmos")) { // Profile: "TrueHD+Atmos / TrueHD"
                audioCodec = "atmos";
            }
        }
    }

    // Custom Logic
    // old <= 18.05 style
    const QString audioCodecUpper = audioCodec.toUpper(); // just for comparison in uppercase
    if (audioCodecUpper == "AC3" || audioCodecUpper == "AC-3") {
        audioCodec = "ac3";

    } else if (audioCodecUpper == "AC3+" || audioCodecUpper == "E-AC-3") {
        audioCodec = "eac3";

    } else if (audioCodecUpper == "FLAC") {
        audioCodec = "flac";

    } else if (audioCodecUpper == "MPA1L3") {
        audioCodec = "mp3";

    } else if (audioCodecUpper == "AAC LC" || audioCodecUpper == "AAC") {
        audioCodec = "aac";
    }

    // Logic similiar to TMM
    // newer 18.12 style
    if ("ac3" == audioCodec || "dts" == audioCodec || "truehd" == audioCodec) {
        const QString formatCommercial = getAudio(streamIndex, "Format_Commercial");
        const QString formatCommercialIfAny = getAudio(streamIndex, "Format_Commercial_IfAny");
        const QString& commercialName = formatCommercial.isEmpty() ? formatCommercialIfAny : formatCommercial;

        if (!commercialName.isEmpty()) {
            if (commercialName.contains("master audio")) {
                audioCodec = "dtshd_ma";

            } else if (commercialName.contains("high resolution audio")) {
                audioCodec = "dtshd_hra";

            } else if (commercialName.contains("extended") || commercialName.contains("es matrix")
                       || commercialName.contains("es discrete")) {
                audioCodec = "dtshd_es";

            } else if (commercialName.contains("atmos")) {
                audioCodec = "atmos";

            } else if (commercialName.contains("ex audio")) {
                // Dolby Digital EX
                audioCodec = "AC3EX";
            }
        }
    }

    if (Settings::instance()->advanced()->audioCodecMappings().contains(audioCodec)) {
        return Settings::instance()->advanced()->audioCodecMappings().value(audioCodec);
    }
    return audioCodec;
}

QString MediaInfoFile::audioChannels(int streamIndex) const
{
    QString channels = getAudio(streamIndex, "Channel(s)");
    QString channelsOriginal = getAudio(streamIndex, "Channel(s)_Original");
    if (!channelsOriginal.isEmpty()) {
        channels = channelsOriginal;
    }
    QRegularExpression rx(R"(^\D*(\d*)\D*)");
    QRegularExpressionMatch match = rx.match(channels);
    channels = match.hasMatch() ? match.captured(1) : "";
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

/**
 * \brief Modifies a video format name
 * \param format Original format, given by libmediainfo
 * \param version Version, given by libmediainfo
 * \return Modified format
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

QStringList MediaInfoFile::getAudio(int streamIndex, QStringList parameters) const
{
    QStringList result;
    for (const QString& parameter : parameters) {
        QString value = getAudio(streamIndex, parameter.toStdString().data());
        if (!value.isEmpty()) {
            result.append(value);
        }
    }
    return result;
}

QString MediaInfoFile::getVideo(int streamIndex, const char* parameter) const
{
    return MI2QString(m_mediaInfo->Get(MediaInfoDLL::Stream_Video, streamIndex, QString2MI(parameter)));
}

QString MediaInfoFile::getText(int streamIndex, const char* parameter) const
{
    return MI2QString(m_mediaInfo->Get(MediaInfoDLL::Stream_Text, streamIndex, QString2MI(parameter)));
}
