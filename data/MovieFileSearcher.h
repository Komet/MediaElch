#ifndef MOVIEFILESEARCHER_H
#define MOVIEFILESEARCHER_H

#include <QObject>
#include <QDir>
#include <QHash>

#include "data/Movie.h"
#include "globals/Globals.h"

/**
 * @brief The MovieFileSearcher class
 */
class MovieFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit MovieFileSearcher(QObject *parent = 0);
    ~MovieFileSearcher();

    void setMovieDirectories(QList<SettingsDir> directories);
    void scanDir(QString startPath, QString path, QList<QStringList> &contents, bool separateFolders = false, bool firstScan = false);

public slots:
    void run();

signals:
    void searchStarted(QString, int);
    void progress(int, int, int);
    void moviesLoaded(int);
    void currentDir(QString);

private:
    QList<SettingsDir> m_directories;
    int m_progressMessageId;
    QStringList getCachedFiles(QString path);
    QHash<QString, QDateTime> m_lastModifications;
};

#endif // MOVIEFILESEARCHER_H
