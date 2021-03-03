#pragma once

#include "globals/Meta.h"
#include "movies/Movie.h"

#include <QDir>
#include <QElapsedTimer>
#include <QHash>
#include <QObject>
#include <QTime>
#include <QVector>
#include <memory>

namespace mediaelch {

class MovieDirectorySearcher;

/// \brief Class responsible for (re-)loading all movies inside given directories.
///
/// \par Example
/// \code{cpp}
///   MovieFileSearcher searcher;
///   searcher.setMovieDirectories(directories);
/// \endcode
class MovieFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit MovieFileSearcher(QObject* parent = nullptr);
    ~MovieFileSearcher() override = default;

    /// \brief Sets the directories to scan for movies. Not readable directories are skipped.
    void setMovieDirectories(const QVector<SettingsDir>& directories);

    /// \brief Scans the given path for movie files.
    ///
    /// Results are in a list which contains a QStringList for every movie.
    ///
    /// \param startPath Scanning started at this path
    /// \param path Path to scan
    /// \param contents List of contents
    /// \param separateFolders Are concerts in separate folders
    /// \param firstScan When this is true, subfolders are scanned, regardless of separateFolders
    /// \deprecated Use reload() instead
    /// \note Only used in MovieFilesOrganizer
    ELCH_DEPRECATED void scanDir(QString startPath,
        QString path,
        QVector<QStringList>& contents,
        bool separateFolders = false,
        bool firstScan = false);

public slots:
    void reload(bool force);
    void abort();

signals:
    void searchStarted(QString);
    void progress(int current, int max, int messageBarId);
    void moviesLoaded();
    void currentDir(QString);

public:
    static void loadMovieData(Movie* movie);

private slots:
    void onDirectoryLoaded(MovieDirectorySearcher* searcher);
    void onDirectoryStartsLoading(int approximateMovieCount);
    void onMovieProcessed(Movie* movie);

private:
    /// \brief Resets all counters, internal variables and so on.
    void resetInternalState();

    /// Get a list of files in a directory
    /// \deprecated Remove with scanDir
    ELCH_DEPRECATED QStringList getFiles(QString path);

private:
    QVector<SettingsDir> m_directories;
    QVector<MovieDirectorySearcher*> m_searchers;
    QElapsedTimer m_reloadTimer;

    /// \deprecated Remove with scanDir
    ELCH_DEPRECATED QHash<QString, QDateTime> m_lastModifications;

    int m_approxMovieSum = 0;
    int m_moviesProcessed = 0;
    int m_directoriesProcessed = 0;

    bool m_running = false;
    bool m_aborted = false;
};

} // namespace mediaelch
