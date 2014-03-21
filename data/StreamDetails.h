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

    virtual QMap<QString, QString> videoDetails();
    virtual QList<QMap<QString, QString> > audioDetails();
    virtual QList<QMap<QString, QString> > subtitleDetails();

private:
    QString videoFormat(QString format, QString version);
    QString audioFormat(const QString &codec, const QString &profile);

    QStringList m_files;
    QMap<QString, QString> m_videoDetails;
    QList<QMap<QString, QString> > m_audioDetails;
    QList<QMap<QString, QString> > m_subtitles;
};

#endif // STREAMDETAILS_H
