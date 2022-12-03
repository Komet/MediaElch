#pragma once

#include "globals/MediaDirectory.h"

#include <QObject>
#include <QString>
#include <QVector>

class Album;
class Artist;

class MusicFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit MusicFileSearcher(QObject* parent = nullptr);
    ~MusicFileSearcher() override = default;

    void setMusicDirectories(QVector<mediaelch::MediaDirectory> directories);
    static Artist* loadArtistData(Artist* artist);
    static Album* loadAlbumData(Album* album);

public slots:
    void reload(bool force);
    void abort();

signals:
    void searchStarted(QString);
    void progress(int, int, int);
    void musicLoaded();
    void currentDir(QString);

private:
    QVector<mediaelch::MediaDirectory> m_directories;
    int m_progressMessageId;
    bool m_aborted;
};
