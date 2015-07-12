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
    void loadStreamDetails();
    void setVideoDetail(QString key, QString value);
    void setAudioDetail(int streamNumber, QString key, QString value);
    void setSubtitleDetail(int streamNumber, QString key, QString value);
    void clear();
    bool hasAudioChannels(int channels);
    bool hasAudioQuality(QString quality);
    int audioChannels();
    QString audioCodec();
    QString videoCodec();

    virtual QMap<QString, QString> videoDetails();
    virtual QList<QMap<QString, QString> > audioDetails();
    virtual QList<QMap<QString, QString> > subtitleDetails();

private:
    QString videoFormat(QString format, QString version);
    QString audioFormat(const QString &codec, const QString &profile);
    QString stereoFormat(const QString &format);
    void loadWithLibrary();

    QStringList m_files;
    QMap<QString, QString> m_videoDetails;
    QList<QMap<QString, QString> > m_audioDetails;
    QList<QMap<QString, QString> > m_subtitles;
    QList<int> m_availableChannels;
    QList<QString> m_availableQualities;

    QStringList m_hdAudioCodecs;
    QStringList m_normalAudioCodecs;
    QStringList m_sdAudioCodecs;
};

#endif // STREAMDETAILS_H
