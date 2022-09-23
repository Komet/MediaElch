#pragma once

#include "globals/Globals.h"

#include <QObject>

class Album;
class Artist;

class MusicFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit MusicFileSearcher(QObject* parent = nullptr);
    ~MusicFileSearcher() override = default;

    void setMusicDirectories(QVector<SettingsDir> directories);
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
    QVector<SettingsDir> m_directories;
    int m_progressMessageId;
    bool m_aborted;
};
