#ifndef STREAMDETAILS_H
#define STREAMDETAILS_H

#include <QMap>
#include <QObject>

#include "data/MediaCenterInterface.h"

/**
 * @brief The StreamDetails class
 *        This class makes use of libstreaminfo and handles
 *        video and audio stream details
 */
class StreamDetails : public QObject
{
    Q_OBJECT
public:
    explicit StreamDetails(QObject *parent, QStringList files);

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

    void loadStreamDetails();
    void setVideoDetail(VideoDetails key, QString value);
    void setAudioDetail(int streamNumber, AudioDetails key, QString value);
    void setSubtitleDetail(int streamNumber, SubtitleDetails key, QString value);
    void clear();
    bool hasAudioChannels(int channels) const;
    bool hasAudioQuality(QString quality) const;
    int audioChannels() const;
    QString audioCodec() const;
    QString videoCodec() const;

    virtual QMap<VideoDetails, QString> videoDetails() const;
    virtual QList<QMap<AudioDetails, QString>> audioDetails() const;
    virtual QList<QMap<SubtitleDetails, QString>> subtitleDetails() const;

private:
    QString videoFormat(QString format, QString version) const;
    QString audioFormat(const QString &codec, const QString &profile) const;
    QString stereoFormat(const QString &format) const;
    void loadWithLibrary();

    QStringList m_files;
    QMap<VideoDetails, QString> m_videoDetails;
    QList<QMap<AudioDetails, QString>> m_audioDetails;
    QList<QMap<SubtitleDetails, QString>> m_subtitles;
    QList<int> m_availableChannels;
    QList<QString> m_availableQualities;

    QStringList m_hdAudioCodecs;
    QStringList m_normalAudioCodecs;
    QStringList m_sdAudioCodecs;
};

#endif // STREAMDETAILS_H
