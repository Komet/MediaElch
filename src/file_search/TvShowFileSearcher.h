#pragma once

#include "data/tv_show/TvShowEpisode.h"
#include "globals/MediaDirectory.h"
#include "media/Path.h"

#include <QDir>
#include <QObject>

class TvShowFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit TvShowFileSearcher(QObject* parent = nullptr);
    void setTvShowDirectories(QVector<mediaelch::MediaDirectory> directories);
    static SeasonNumber getSeasonNumber(QStringList files);
    static QVector<EpisodeNumber> getEpisodeNumbers(QStringList files);
    static TvShowEpisode* loadEpisodeData(TvShowEpisode* episode);
    static TvShowEpisode* reloadEpisodeData(TvShowEpisode* episode);

public slots:
    void reload(bool force);
    void reloadEpisodes(const mediaelch::DirectoryPath& showDir);
    void abort();

signals:
    void searchStarted(QString);
    void progress(int, int, int);
    void tvShowsLoaded();
    void currentDir(QString);

private:
    QVector<mediaelch::MediaDirectory> m_directories;
    int m_progressMessageId;
    void getTvShows(const mediaelch::DirectoryPath& path, QMap<QString, QVector<QStringList>>& contents);
    void scanTvShowDir(const mediaelch::DirectoryPath& startPath,
        const mediaelch::DirectoryPath& path,
        QVector<QStringList>& contents);
    QStringList getFiles(const mediaelch::DirectoryPath& path);
    bool m_aborted;

private:
    void clearOldTvShows(bool forceClear);
    /// \brief Get a map of TV show paths and their respective files in the show folder.
    QMap<QString, QVector<QStringList>> readTvShowContent(bool forceReload);
    QVector<TvShow*> getShowsFromDatabase(bool forceReload);
    void setupShows(QMap<QString, QVector<QStringList>>& contents, int& episodeCounter, int episodeSum);
    void setupShowsFromDatabase(QVector<TvShow*>& dbShows, int episodeCounter, int episodeSum);
};
