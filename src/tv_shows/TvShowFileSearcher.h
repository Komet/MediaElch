#pragma once

#include "tv_shows/TvShowEpisode.h"

#include <QDir>
#include <QObject>

class Database;

class TvShowFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit TvShowFileSearcher(QObject* parent = nullptr);
    void setTvShowDirectories(QVector<SettingsDir> directories);
    static SeasonNumber getSeasonNumber(QStringList files);
    static QVector<EpisodeNumber> getEpisodeNumbers(QStringList files);
    static TvShowEpisode* loadEpisodeData(TvShowEpisode* episode);
    static TvShowEpisode* reloadEpisodeData(TvShowEpisode* episode);

public slots:
    void reload(bool force);
    void reloadEpisodes(QString showDir);
    void abort();

signals:
    void searchStarted(QString);
    void progress(int, int, int);
    void tvShowsLoaded();
    void currentDir(QString);

private:
    QVector<SettingsDir> m_directories;
    int m_progressMessageId;
    void getTvShows(QString path, QMap<QString, QVector<QStringList>>& contents);
    void scanTvShowDir(QString startPath, QString path, QVector<QStringList>& contents);
    QStringList getFiles(QString path);
    bool m_aborted;

private:
    Database& database();

    void clearOldTvShows(bool forceClear);
    QMap<QString, QVector<QStringList>> readTvShowContent(bool forceReload);
    QVector<TvShow*> getShowsFromDatabase(bool forceReload);
    void setupShows(QMap<QString, QVector<QStringList>>& contents, int& episodeCounter, int episodeSum);
    void setupShowsFromDatabase(QVector<TvShow*>& dbShows, int episodeCounter, int episodeSum);
};
