#ifndef TVSHOWFILESEARCHER_H
#define TVSHOWFILESEARCHER_H

#include "data/TvShowEpisode.h"
#include "globals/Globals.h"

#include <QDir>
#include <QObject>

/**
 * @brief The TvShowFileSearcher class
 */
class TvShowFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit TvShowFileSearcher(QObject *parent = nullptr);
    void setMovieDirectories(QList<SettingsDir> directories);
    static int getSeasonNumber(QStringList files);
    static QList<EpisodeNumber> getEpisodeNumbers(QStringList files);
    static TvShowEpisode *loadEpisodeData(TvShowEpisode *episode);
    static TvShowEpisode *reloadEpisodeData(TvShowEpisode *episode);

public slots:
    void reload(bool force);
    void reloadEpisodes(QString showDir);
    void abort();

signals:
    void searchStarted(QString, int);
    void progress(int, int, int);
    void tvShowsLoaded(int);
    void currentDir(QString);

private:
    QList<SettingsDir> m_directories;
    int m_progressMessageId;
    void getTvShows(QString path, QMap<QString, QList<QStringList>> &contents);
    void scanTvShowDir(QString startPath, QString path, QList<QStringList> &contents);
    QStringList getFiles(QString path);
    bool m_aborted;
};

#endif // TVSHOWFILESEARCHER_H
