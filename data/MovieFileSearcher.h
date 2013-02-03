#ifndef MOVIEFILESEARCHER_H
#define MOVIEFILESEARCHER_H

#include <QObject>
#include <QDir>
#include <QHash>
#include <QTime>

#include "movies/Movie.h"

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
    void reload(bool force);
    void abort();

signals:
    void searchStarted(QString, int);
    void progress(int, int, int);
    void moviesLoaded(int);
    void currentDir(QString);

private:
    QStringList getFiles(QString path);

    QList<SettingsDir> m_directories;
    int m_progressMessageId;
    QHash<QString, QDateTime> m_lastModifications;
    bool m_aborted;

    struct MovieContents {
        QString path;
        bool inSeparateFolder;
        QMap<QString, QStringList> contents;
    };
};

#endif // MOVIEFILESEARCHER_H
