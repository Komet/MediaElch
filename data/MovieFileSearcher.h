#ifndef MOVIEFILESEARCHER_H
#define MOVIEFILESEARCHER_H

#include <QThread>
#include <QDir>

#include "data/Movie.h"
#include "globals/Globals.h"

/**
 * @brief The MovieFileSearcher class
 */
class MovieFileSearcher : public QThread
{
    Q_OBJECT
public:
    explicit MovieFileSearcher(QObject *parent = 0);
    ~MovieFileSearcher();

    void run();
    void setMovieDirectories(QList<SettingsDir> directories);

signals:
    void searchStarted(QString, int);
    void progress(int, int, int);
    void moviesLoaded(int);

private:
    QList<SettingsDir> m_directories;
    int m_progressMessageId;
    void scanDir(QString path, QList<QStringList> &contents, bool separateFolders = false, bool firstScan = false);
    QStringList getCachedFiles(QString path);
};

#endif // MOVIEFILESEARCHER_H
