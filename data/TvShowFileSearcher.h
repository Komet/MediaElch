#ifndef TVSHOWFILESEARCHER_H
#define TVSHOWFILESEARCHER_H

#include <QDir>
#include <QObject>
#include "globals/Globals.h"

/**
 * @brief The TvShowFileSearcher class
 */
class TvShowFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit TvShowFileSearcher(QObject *parent = 0);
    void setMovieDirectories(QList<SettingsDir> directories);

public slots:
    void run();

signals:
    void searchStarted(QString, int);
    void progress(int, int, int);
    void tvShowsLoaded(int);
    void currentDir(QString);

private:
    QStringList m_directories;
    int m_progressMessageId;
    void getTvShows(QString path, QMap<QString, QList<QStringList> > &contents);
    void scanTvShowDir(QString startPath, QString path, QList<QStringList> &contents);
    QStringList getCachedFiles(QString path);
};

#endif // TVSHOWFILESEARCHER_H
