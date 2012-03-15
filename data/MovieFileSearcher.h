#ifndef MOVIEFILESEARCHER_H
#define MOVIEFILESEARCHER_H

#include <QThread>
#include <QDir>

#include "data/Movie.h"

class MovieFileSearcher : public QThread
{
    Q_OBJECT
public:
    explicit MovieFileSearcher(QObject *parent = 0);
    ~MovieFileSearcher();

    void run();
    void setMovieDirectories(QStringList directories);

signals:
    void searchStarted(QString);
    void progress(int, int);
    void moviesLoaded();

private:
    QStringList m_directories;

    void getDirContents(QString path, QList<QStringList> &contents);
};

#endif // MOVIEFILESEARCHER_H
