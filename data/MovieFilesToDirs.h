#ifndef MOVIEFILESTODIRS_H
#define MOVIEFILESTODIRS_H

#include <QThread>
#include <QDir>

#include "data/Movie.h"
#include "globals/Globals.h"

/**
 * @brief The MovieFilesToDirs class
 */
class MovieFilesToDirs : public QThread
{
    Q_OBJECT
public:
    explicit MovieFilesToDirs(QObject *parent = 0);
    ~MovieFilesToDirs();

    void run();
    //void setMovieDirectories(QList<SettingsDir> directories);

signals:
    void progressStarted(QString, int);
    //void progress(int, int, int);
    //void moviesLoaded(int);

private:
    //QList<SettingsDir> m_directories;
    int m_progressMessageId;
    //void getDirContents(QString path, QList<QStringList> &contents);
};

#endif MOVIEFILESTODIRS_H

