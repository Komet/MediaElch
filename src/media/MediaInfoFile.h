#pragma once

#include "media/Path.h"

#include <QString>
#include <chrono>
#include <memory>

namespace MediaInfoDLL {
class MediaInfo;
}


// Note: Normally, on Windows I'd expect MediaInfoDLL to use std::wstring.
// But for some reason, that may not be the case, so we can't use Q_OS_WIN
// to check for that.  With SFINAE, we can still make it work.

template<typename T>
static typename std::enable_if_t<std::is_same<char, T>::value, std::string> toMediaInfoString(const QString& str)
{
    return std::string(str.toUtf8().data()); // TODO: Can we use toStdString()?
}

template<typename T>
static typename std::enable_if_t<std::is_same<wchar_t, T>::value, std::wstring> toMediaInfoString(const QString& str)
{
    return std::wstring(str.toStdWString());
}

template<typename T>
static QString fromMediaInfoString(typename std::enable_if_t<std::is_same<char, T>::value, std::string> str)
{
    return QString::fromStdString(str);
}

template<typename T>
static QString fromMediaInfoString(typename std::enable_if_t<std::is_same<wchar_t, T>::value, std::wstring> str)
{
    return QString::fromStdWString(str);
}


#define MI2QString(_DATA) fromMediaInfoString<MediaInfoDLL::Char>(_DATA)
#define QString2MI(_DATA) toMediaInfoString<MediaInfoDLL::Char>(_DATA)


/// Represents a single media file whose information we want to get using MediaInfo.
/// Essentially just a wrapper with some sanity checks.
class MediaInfoFile
{
public:
    MediaInfoFile(const QString& filepath);
    ~MediaInfoFile();

    static bool hasMediaInfo();

    ELCH_NODISCARD bool isReady() const;

    ELCH_NODISCARD int subtitleCount() const;
    ELCH_NODISCARD int videoStreamCount() const;
    ELCH_NODISCARD int audioStreamCount() const;

    std::chrono::milliseconds duration(int streamIndex) const;
    std::size_t videoWidth(int streamIndex) const;
    std::size_t videoHeight(int streamIndex) const;
    double aspectRatio(int streamIndex) const;
    ELCH_NODISCARD QString codec(int streamIndex) const;
    ELCH_NODISCARD QString mpegVersion(int streamIndex) const;
    ELCH_NODISCARD QString scanType(int streamIndex) const;
    ELCH_NODISCARD QString stereoFormat(int streamIndex) const;
    ELCH_NODISCARD QString format(int streamIndex) const;
    ELCH_NODISCARD QString hdrType(int streamIndex) const;

    ELCH_NODISCARD QString audioLanguage(int streamIndex) const;
    ELCH_NODISCARD QString audioCodec(int streamIndex) const;
    ELCH_NODISCARD QString audioChannels(int streamIndex) const;

    ELCH_NODISCARD QString subtitleLang(int streamIndex) const;

private:
    ELCH_NODISCARD QString parseVideoFormat(QString format, QString version) const;

    ELCH_NODISCARD QString getGeneral(int streamIndex, const char* parameter) const;
    ELCH_NODISCARD QString getVideo(int streamIndex, const char* parameter) const;
    ELCH_NODISCARD QString getAudio(int streamIndex, const char* parameter) const;
    ELCH_NODISCARD QStringList getAudio(int streamIndex, QStringList parameters) const;
    ELCH_NODISCARD QString getText(int streamIndex, const char* parameter) const;

private:
    // We don't want the MediaInfoLib include here so we avoid it by
    // using the forward declaration of the class MediaInfo
    std::unique_ptr<MediaInfoDLL::MediaInfo> m_mediaInfo;
    bool m_isReady = false;
};
