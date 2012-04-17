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
    void searchStarted(QString, int);
    void progress(int, int, int);
    void moviesLoaded(int);

private:
    QStringList m_directories;
    int m_progressMessageId;
    void getDirContents(QString path, QList<QStringList> &contents);
};

#endif // MOVIEFILESEARCHER_H
