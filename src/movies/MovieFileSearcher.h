#pragma once

#include "movies/Movie.h"

#include <QDir>
#include <QHash>
#include <QObject>
#include <QTime>
#include <QVector>
#include <memory>

/// MovieFileSearcher is responsible for (re-)loading all movie inside
/// given directories.
///
/// @example
///   MovieFileSearcher searcher;
///   searcher.setMovieDirectories(directories);
///
class MovieFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit MovieFileSearcher(QObject* parent = nullptr);
    ~MovieFileSearcher() override = default;

    void setMovieDirectories(const QVector<SettingsDir>& directories);

    void scanDir(QString startPath,
        QString path,
        QVector<QStringList>& contents,
        bool separateFolders = false,
        bool firstScan = false);

public slots:
    void reload(bool force);
    void abort();

signals:
    void searchStarted(QString);
    void progress(int, int, int);
    void moviesLoaded();
    void currentDir(QString);

private:
    struct MovieContents
    {
        QString path;
        bool inSeparateFolder;
        QMap<QString, QStringList> contents;
    };

    static Movie* loadMovieData(Movie* movie);

    QStringList getFiles(QString path);

    int loadMoviesFromDirectory(const SettingsDir& movieDir,
        bool force,
        QVector<MovieContents>& moviesContent,
        QVector<Movie*>& dbMovies,
        QStringList& bluRays,
        QStringList& dvds);
    QVector<Movie*> loadAndStoreMoviesContents(QVector<MovieContents>& moviesContent,
        QStringList& bluRays,
        QStringList& dvds,
        int& movieSum,
        int& movieCounter);

    QVector<SettingsDir> m_directories;
    int m_progressMessageId;
    QHash<QString, QDateTime> m_lastModifications;
    bool m_aborted;
};
