#ifndef MUSICFILESEARCHER_H
#define MUSICFILESEARCHER_H

#include <QObject>

#include "../globals/Globals.h"
#include "../music/Album.h"
#include "../music/Artist.h"

class MusicFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit MusicFileSearcher(QObject *parent = 0);
    ~MusicFileSearcher();

    void setMusicDirectories(QList<SettingsDir> directories);
    static Artist *loadArtistData(Artist *artist);
    static Album *loadAlbumData(Album *album);

public slots:
    void reload(bool force);
    void abort();

signals:
    void searchStarted(QString, int);
    void progress(int, int, int);
    void musicLoaded(int);
    void currentDir(QString);

private:
    QList<SettingsDir> m_directories;
    int m_progressMessageId;
    bool m_aborted;
};

#endif // MUSICFILESEARCHER_H
