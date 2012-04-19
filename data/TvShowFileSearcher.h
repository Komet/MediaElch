#ifndef TVSHOWFILESEARCHER_H
#define TVSHOWFILESEARCHER_H

#include <QDir>
#include <QThread>
#include "Globals.h"

class TvShowFileSearcher : public QThread
{
    Q_OBJECT
public:
    explicit TvShowFileSearcher(QObject *parent = 0);
    void run();
    void setMovieDirectories(QStringList directories);

signals:
    void searchStarted(QString, int);
    void progress(int, int, int);
    void tvShowsLoaded(int);

private:
    QStringList m_directories;
    int m_progressMessageId;
    void getDirContents(QString path, QMap<QString, QList<QStringList> > &contents);
};

#endif // TVSHOWFILESEARCHER_H
