#include "media/StreamDetails.h"

#include "log/Log.h"
#include "media/MediaInfoFile.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>

StreamDetails::StreamDetails(QObject* parent, mediaelch::FileList files) :
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
    case VideoDetails::Unknown: return "undefined";
    case VideoDetails::Codec: return "codec";
    case VideoDetails::Aspect: return "aspect";
    case VideoDetails::Width: return "width";
    case VideoDetails::Height: return "height";
    case VideoDetails::DurationInSeconds: return "durationinseconds";
    case VideoDetails::ScanType: return "scantype";
    case VideoDetails::StereoMode: return "stereomode";
    case VideoDetails::HdrType: return "hdrtype";
    }
    qCWarning(generic) << "Undefined video detail: no string representation";
    return "undefined";
}

QString StreamDetails::detailToString(AudioDetails details)
{
    switch (details) {
    case AudioDetails::Unknown: return "undefined";
    case AudioDetails::Codec: return "codec";
    case AudioDetails::Language: return "language";
    case AudioDetails::Channels: return "channels";
    }
    qCWarning(generic) << "Undefined audio detail: no string representation";
    return "undefined";
}

QString StreamDetails::detailToString(SubtitleDetails details)
{
    switch (details) {
    case SubtitleDetails::Unknown: return "undefined";
    case SubtitleDetails::Language: return "language";
    }
    qCWarning(generic) << "Undefined subtitle detail: no string representation";
    return "undefined";
}

StreamDetails::VideoDetails StreamDetails::stringToVideoDetail(QString detail)
{
    static const QMap<QString, StreamDetails::VideoDetails> map = {
        {"codec", StreamDetails::VideoDetails::Codec},
        {"aspect", StreamDetails::VideoDetails::Aspect},
        {"width", StreamDetails::VideoDetails::Width},
        {"height", StreamDetails::VideoDetails::Height},
        {"durationinseconds", StreamDetails::VideoDetails::DurationInSeconds},
        {"scantype", StreamDetails::VideoDetails::ScanType},
        {"stereomode", StreamDetails::VideoDetails::StereoMode},
        {"hdrtype", StreamDetails::VideoDetails::HdrType},
    };
    if (map.contains(detail)) {
        return map[detail];
    }

    return StreamDetails::VideoDetails::Unknown;
}

StreamDetails::AudioDetails StreamDetails::stringToAudioDetail(QString detail)
{
    static const QMap<QString, StreamDetails::AudioDetails> map = {
        {"codec", StreamDetails::AudioDetails::Codec},
        {"language", StreamDetails::AudioDetails::Language},
        {"channels", StreamDetails::AudioDetails::Channels},
    };
    if (map.contains(detail)) {
        return map[detail];
    }

    return StreamDetails::AudioDetails::Unknown;
}

StreamDetails::SubtitleDetails StreamDetails::stringToSubtitleDetail(QString detail)
{
    static const QMap<QString, StreamDetails::SubtitleDetails> map = {
        {"language", StreamDetails::SubtitleDetails::Language},
    };
    if (map.contains(detail)) {
        return map[detail];
    }

    return StreamDetails::SubtitleDetails::Unknown;
}

void StreamDetails::clear()
{
    m_videoDetails.clear();
    m_audioDetails.clear();
    m_subtitles.clear();
    m_availableChannels.clear();
    m_availableQualities.clear();
}

bool StreamDetails::loadStreamDetails()
{
    m_hasLoadedStreamDetails = true;
    if (m_files.isEmpty()) {
        return false;
    }
    const QString firstFile = m_files.first().toString();
    if (firstFile.endsWith(".iso", Qt::CaseInsensitive) || firstFile.endsWith(".img", Qt::CaseInsensitive)) {
        // MediaInfo does not work with ISOs of BluRays, etc.
        return false;
    }

    // If it's a DVD structure, compute the biggest part (main movie) and use this IFO file
    if (firstFile.endsWith("VIDEO_TS.IFO")) {
        QMap<QString, qint64> sizes;
        QString biggest;
        qint64 biggestSize = 0;
        QFileInfo fi(firstFile);
        const auto entries = fi.dir().entryInfoList(QStringList{"VTS_*.VOB", "vts_*.vob"}, QDir::Files, QDir::Name);
        for (const QFileInfo& fiVob : entries) {
            QRegularExpression rx("VTS_([0-9]*)_[0-9]*.VOB",
                QRegularExpression::InvertedGreedinessOption | QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = rx.match(fiVob.fileName());
            if (match.hasMatch()) {
                if (!sizes.contains(match.captured(1))) {
                    sizes.insert(match.captured(1), 0);
                }
                sizes[match.captured(1)] += fiVob.size();
                if (sizes[match.captured(1)] > biggestSize) {
                    biggestSize = sizes[match.captured(1)];
                    biggest = match.captured(1);
                }
            }
        }
        if (!biggest.isEmpty()) {
            QFileInfo fiNew(fi.absolutePath() + "/VTS_" + biggest + "_0.IFO");
            if (fiNew.isFile() && fiNew.exists()) {
                m_files = mediaelch::FileList({mediaelch::FilePath(fiNew.absoluteFilePath())});
            }
        }
    }

    return loadWithLibrary();
}

void StreamDetails::setLoaded(bool loaded)
{
    m_hasLoadedStreamDetails = loaded;
}

void StreamDetails::setFilesWithoutReloading(mediaelch::FileList files)
{
    m_files = std::move(files);
}

bool StreamDetails::hasLoaded() const
{
    return m_hasLoadedStreamDetails;
}

bool StreamDetails::loadWithLibrary()
{
    mediaelch::FilePath filePath = m_files.first();
    if (m_files.size() == 1 && filePath.toString().endsWith("index.bdmv")) {
        QFileInfo fi(filePath.toString());
        QDir dir(fi.absolutePath() + "/STREAM");
        QStringList files = dir.entryList(QStringList() << "*.m2ts", QDir::NoDotAndDotDot | QDir::Files, QDir::Name);
        if (!files.isEmpty()) {
            filePath = mediaelch::FilePath(dir.absolutePath() + "/" + files.first());
        }
    }

    MediaInfoFile mi(filePath.toString());

    // It could be that MediaInfo wasn't loaded successfully.
    if (!mi.isReady()) {
        return false;
    }

    clear();

    std::chrono::seconds duration{0};

    if (m_files.size() > 1) {
        for (const mediaelch::FilePath& file : m_files) {
            const MediaInfoFile mediaFile(file.toString());
            // cast is fine here; if there indeed are files that long, loosing precision is not too bad.
            duration += std::chrono::seconds(qRound(static_cast<double>(mediaFile.duration(0).count()) / 1000.));
        }
    } else {
        // cast is fine here; if there indeed are files that long, loosing precision is not too bad.
        duration += std::chrono::seconds(qRound(static_cast<double>(mi.duration(0).count()) / 1000.));
    }

    setVideoDetail(StreamDetails::VideoDetails::DurationInSeconds, QString::number(duration.count()));

    if (mi.videoStreamCount() > 0) {
        setVideoDetail(VideoDetails::Codec, mi.format(0));
        setVideoDetail(VideoDetails::Aspect, QString::number(mi.aspectRatio(0)));
        setVideoDetail(VideoDetails::Width, QString::number(mi.videoWidth(0)));
        setVideoDetail(VideoDetails::Height, QString::number(mi.videoHeight(0)));
        setVideoDetail(VideoDetails::ScanType, mi.scanType(0));
        setVideoDetail(VideoDetails::StereoMode, mi.stereoFormat(0));
        setVideoDetail(VideoDetails::HdrType, mi.hdrType(0));
    }

    const int audioCount = mi.audioStreamCount();
    for (int i = 0; i < audioCount; ++i) {
        setAudioDetail(i, AudioDetails::Language, mi.audioLanguage(i));
        setAudioDetail(i, AudioDetails::Codec, mi.audioCodec(i));
        setAudioDetail(i, AudioDetails::Channels, mi.audioChannels(i));
    }

    int textCount = mi.subtitleCount();
    for (int i = 0; i < textCount; ++i) {
        setSubtitleDetail(i, StreamDetails::SubtitleDetails::Language, mi.subtitleLang(i));
    }
    return true;
}


/**
 * \brief Sets a video detail
 * \param key The key (aspect, width, height...)
 * \param value The value
 */
void StreamDetails::setVideoDetail(VideoDetails key, QString value)
{
    m_videoDetails.insert(key, value);
}

/**
 * \brief Sets a audio detail
 * \param streamNumber Number of the stream
 * \param key Key (language, codec or channels)
 * \param value Value
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
 * \brief Sets a subtitle detail
 * \param streamNumber Number of the stream
 * \param key Key (language)
 * \param value Language
 */
void StreamDetails::setSubtitleDetail(int streamNumber, SubtitleDetails key, QString value)
{
    if (streamNumber >= m_subtitles.count()) {
        m_subtitles.resize(streamNumber);
        m_subtitles.insert(streamNumber, QMap<SubtitleDetails, QString>{{key, value}});
        return;
    }
    m_subtitles[streamNumber].insert(key, value);
}

/**
 * \brief Access video details
 */
QMap<StreamDetails::VideoDetails, QString> StreamDetails::videoDetails() const
{
    return m_videoDetails;
}

QVector<QMap<StreamDetails::AudioDetails, QString>> StreamDetails::audioDetails() const
{
    return m_audioDetails;
}

QVector<QMap<StreamDetails::SubtitleDetails, QString>> StreamDetails::subtitleDetails() const
{
    return m_subtitles;
}

QStringList StreamDetails::allAudioLanguages() const
{
    QStringList languages;
    for (const auto& audioDetail : m_audioDetails) {
        if (audioDetail.contains(AudioDetails::Language)) {
            languages << audioDetail[AudioDetails::Language];
        }
    }
    return languages;
}

QStringList StreamDetails::allSubtitleLanguages() const
{
    QStringList languages;
    for (const auto& subtitleDetail : m_subtitles) {
        if (subtitleDetail.contains(SubtitleDetails::Language)) {
            languages << subtitleDetail[SubtitleDetails::Language];
        }
    }
    return languages;
}

bool StreamDetails::hasAudioChannels(int channels) const
{
    return m_availableChannels.contains(channels);
}

bool StreamDetails::hasAudioQuality(QString quality) const
{
    return m_availableQualities.contains(quality);
}

bool StreamDetails::hasSubtitles() const
{
    return !m_subtitles.isEmpty();
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
    QString defaultCodec;
    for (elch_ssize_t i = 0, n = m_audioDetails.count(); i < n; ++i) {
        QString codec = m_audioDetails.at(i).value(AudioDetails::Codec);
        if (m_hdAudioCodecs.contains(codec)) {
            hdCodec = codec;
        } else if (m_normalAudioCodecs.contains(codec)) {
            normalCodec = codec;
        } else if (m_sdAudioCodecs.contains(codec)) {
            sdCodec = codec;
        } else {
            defaultCodec = codec;
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
    return defaultCodec;
}

QString StreamDetails::videoCodec() const
{
    return m_videoDetails.value(VideoDetails::Codec);
}

QVector<StreamDetails::VideoDetails> StreamDetails::allVideoDetailsAsList()
{
    return {
        StreamDetails::VideoDetails::Codec,
        StreamDetails::VideoDetails::Aspect,
        StreamDetails::VideoDetails::Width,
        StreamDetails::VideoDetails::Height,
        StreamDetails::VideoDetails::DurationInSeconds,
        StreamDetails::VideoDetails::ScanType,
        StreamDetails::VideoDetails::StereoMode,
        StreamDetails::VideoDetails::HdrType,
    };
}

QVector<StreamDetails::AudioDetails> StreamDetails::allAudioDetailsAsList()
{
    return {
        StreamDetails::AudioDetails::Codec,
        StreamDetails::AudioDetails::Language,
        StreamDetails::AudioDetails::Channels,
    };
}

QVector<StreamDetails::SubtitleDetails> StreamDetails::allSubtitleDetailsAsList()
{
    return {
        StreamDetails::SubtitleDetails::Language,
    };
}

QMap<QString, QString> StreamDetails::stereoModes()
{
    QMap<QString, QString> modes;
    modes.insert("left_right", "side by side (left eye first)");
    modes.insert("bottom_top", "top-bottom (right eye first)");
    modes.insert("bottom_top", "top-bottom (left eye first)");
    modes.insert("checkerboard_rl", "checkboard (right eye first)");
    modes.insert("checkerboard_lr", "checkboard (left eye first)");
    modes.insert("row_interleaved_rl", "row interleaved (right eye first)");
    modes.insert("row_interleaved_lr", "row interleaved (left eye first)");
    modes.insert("col_interleaved_rl", "column interleaved (right eye first)");
    modes.insert("col_interleaved_lr", "column interleaved (left eye first)");
    modes.insert("anaglyph_cyan_red", "anaglyph (cyan/red)");
    modes.insert("right_left", "side by side (right eye first)");
    modes.insert("anaglyph_green_magenta", "anaglyph (green/magenta)");
    modes.insert("block_lr", "both eyes laced in one block (left eye first)");
    modes.insert("block_rl", "both eyes laced in one block (right eye first)");
    return modes;
}

QVector<QString> StreamDetails::hdrTypes()
{
    QVector<QString> types{
        "hdr10",
        "dolbyvision",
        "hlg",
    };
    return types;
}