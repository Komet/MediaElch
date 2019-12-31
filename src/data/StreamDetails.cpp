#include "StreamDetails.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include "data/MediaInfoFile.h"

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
    QString fileName = m_files.first();
    if (m_files.count() == 1 && m_files.first().endsWith("index.bdmv")) {
        QFileInfo fi(fileName);
        QDir dir(fi.absolutePath() + "/STREAM");
        QStringList files = dir.entryList(QStringList() << "*.m2ts", QDir::NoDotAndDotDot | QDir::Files, QDir::Name);
        if (!files.isEmpty()) {
            fileName = dir.absolutePath() + "/" + files.first();
        }
    }

    MediaInfoFile mi(fileName);

    std::chrono::seconds duration{0};
    QString scanType;

    if (m_files.count() > 1) {
        for (const QString& file : m_files) {
            duration += std::chrono::seconds(qRound(MediaInfoFile(file).duration(0).count() / 1000.));
        }
    } else {
        duration += std::chrono::seconds(qRound(mi.duration(0).count() / 1000.));
    }

    setVideoDetail(StreamDetails::VideoDetails::DurationInSeconds, QString::number(duration.count()));

    if (mi.videoStreamCount() > 0) {
        setVideoDetail(VideoDetails::Codec, mi.format(0));
        setVideoDetail(VideoDetails::Aspect, QString::number(mi.aspectRatio(0)));
        setVideoDetail(VideoDetails::Width, QString::number(mi.videoWidth(0)));
        setVideoDetail(VideoDetails::Height, QString::number(mi.videoHeight(0)));
        setVideoDetail(VideoDetails::ScanType, mi.scanType(0));
        setVideoDetail(VideoDetails::StereoMode, mi.stereoFormat(0));
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
        m_subtitles.resize(streamNumber);
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

QVector<QMap<StreamDetails::AudioDetails, QString>> StreamDetails::audioDetails() const
{
    return m_audioDetails;
}

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
