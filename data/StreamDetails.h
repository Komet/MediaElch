#ifndef STREAMDETAILS_H
#define STREAMDETAILS_H

#include <QMap>
#include <QObject>
#include "data/MediaCenterInterface.h"

class StreamDetails : public QObject
{
    Q_OBJECT
public:
    explicit StreamDetails(QObject *parent, QString file);
    void loadStreamDetails();
    void setVideoDetail(QString key, QString value);
    void setAudioDetail(int streamNumber, QString key, QString value);
    void setSubtitleDetail(int streamNumber, QString key, QString value);
    void clear();

    QMap<QString, QString> videoDetails();
    QList<QMap<QString, QString> > audioDetails();
    QList<QMap<QString, QString> > subtitleDetails();

private:
    QString videoFormat(QString format, QString version);
    QString audioFormat(QString format);

    QString m_file;
    QMap<QString, QString> m_videoDetails;
    QList<QMap<QString, QString> > m_audioDetails;
    QList<QMap<QString, QString> > m_subtitles;
};

#endif // STREAMDETAILS_H
