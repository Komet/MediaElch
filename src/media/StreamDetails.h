#pragma once

#include "media/Path.h"

#include <QMap>
#include <QObject>
#include <QVector>

/// This class makes use of libmediainfo and handles
/// video and audio stream details as well as subtitles
class StreamDetails : public QObject
{
    Q_OBJECT
public:
    explicit StreamDetails(QObject* parent, mediaelch::FileList files);

    enum class VideoDetails
    {
        DurationInSeconds,
        Codec,
        Aspect,
        Width,
        Height,
        ScanType,
        StereoMode
    };
    enum class AudioDetails
    {
        Language,
        Codec,
        Channels
    };
    enum class SubtitleDetails
    {
        Language
    };

    static QString detailToString(VideoDetails details);
    static QString detailToString(AudioDetails details);
    static QString detailToString(SubtitleDetails details);

    /// \brief Loads stream details from the file. Returns true if successful.
    ELCH_NODISCARD bool loadStreamDetails();
    /// \brief Indicates whether the stream details were loaded at least once.
    /// \details Returns true, f the stream details were either loaded through
    ///          \see loadStreamDetails or if set through \see setLoaded.
    ELCH_NODISCARD bool hasLoaded() const;
    /// \brief Explicitly state that the streamdetails were loaded, e.g. through reading the NFO file.
    void setLoaded(bool loaded);
    /// \brief Set the list of files without reloading/changing the current streamdetails.
    /// \details Useful if files were renamed, but their contents did not change.
    void setFilesWithoutReloading(mediaelch::FileList files);

    void setVideoDetail(VideoDetails key, QString value);
    void setAudioDetail(int streamNumber, AudioDetails key, QString value);
    void setSubtitleDetail(int streamNumber, SubtitleDetails key, QString value);
    void clear();
    bool hasAudioChannels(int channels) const;
    bool hasAudioQuality(QString quality) const;
    bool hasSubtitles() const;
    int audioChannels() const;
    QString audioCodec() const;
    QString videoCodec() const;

    virtual QMap<VideoDetails, QString> videoDetails() const;
    virtual QVector<QMap<AudioDetails, QString>> audioDetails() const;
    virtual QVector<QMap<SubtitleDetails, QString>> subtitleDetails() const;

    /// \brief Returns a list of all audio languages available.
    /// \details The returned list is sorted by audio channel.
    QStringList allAudioLanguages() const;
    /// \brief Returns a list of all subtitle languages available.
    QStringList allSubtitleLanguages() const;

private:
    bool loadWithLibrary();

    mediaelch::FileList m_files;
    QMap<VideoDetails, QString> m_videoDetails;
    QVector<QMap<AudioDetails, QString>> m_audioDetails;
    QVector<QMap<SubtitleDetails, QString>> m_subtitles;
    QVector<int> m_availableChannels;
    QVector<QString> m_availableQualities;

    QStringList m_hdAudioCodecs;
    QStringList m_normalAudioCodecs;
    QStringList m_sdAudioCodecs;

    bool m_hasLoadedStreamDetails{false};
};
