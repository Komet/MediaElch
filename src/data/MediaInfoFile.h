#pragma once

#include <QString>
#include <chrono>
#include <memory>

namespace MediaInfoDLL {
class MediaInfo;
}

/// Represents a single media file whose information we want to get using MediaInfo.
/// Essentially just a wrapper with some sanity checks.
class MediaInfoFile
{
public:
    MediaInfoFile(const QString& filepath);
    ~MediaInfoFile();

    int subtitleCount() const;
    int videoStreamCount() const;
    int audioStreamCount() const;

    std::chrono::milliseconds duration(int streamIndex) const;
    std::size_t videoWidth(int streamIndex) const;
    std::size_t videoHeight(int streamIndex) const;
    double aspectRatio(int streamIndex) const;
    QString codec(int streamIndex) const;
    QString mpegVersion(int streamIndex) const;
    QString scanType(int streamIndex) const;
    QString stereoFormat(int streamIndex) const;
    QString format(int streamIndex) const;

    QString audioLanguage(int streamIndex) const;
    QString audioCodec(int streamIndex) const;
    QString audioChannels(int streamIndex) const;

    QString subtitleLang(int streamIndex) const;

private:
    QString parseVideoFormat(QString format, QString version) const;

    QString getGeneral(int streamIndex, const char* parameter) const;
    QString getVideo(int streamIndex, const char* parameter) const;
    QString getAudio(int streamIndex, const char* parameter) const;
    QStringList getAudio(int streamIndex, QStringList parameters) const;
    QString getText(int streamIndex, const char* parameter) const;

private:
    // We don't want the MediaInfoLib include here so we avoid it by
    // using the forward declaration of the class MediaInfo
    std::unique_ptr<MediaInfoDLL::MediaInfo> m_mediaInfo;
};
