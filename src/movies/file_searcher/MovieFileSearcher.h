#pragma once

#include "movies/Movie.h"

#include <QDir>
#include <QElapsedTimer>
#include <QHash>
#include <QObject>
#include <QTime>
#include <QVector>
#include <memory>

namespace mediaelch {

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
    Q_DECL_DEPRECATED void scanDir(QString startPath,
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

    // private signals

    void directoriesLoaded();

private:
    struct MovieContents
    {
        QString path;
        bool inSeparateFolder;
        QMap<QString, QStringList> contents;
    };

    static void loadMovieData(Movie* movie);

private slots:
    void onDirectoriesLoaded();

private:
    /// \brief Resets all counters, internal variables and so on.
    void resetInternalState();
    QStringList getFiles(QString path);

    int loadMoviesContentFromDirectory(const SettingsDir& movieDir, bool force);

    QVector<Movie*> loadAndStoreMoviesContents(int& movieCounter);

private:
    QVector<SettingsDir> m_directories;
    int m_progressMessageId;
    QHash<QString, QDateTime> m_lastModifications;

    QVector<MovieContents> m_movieDirectoriesContent;
    QVector<Movie*> m_dbMovies;
    QStringList m_bluRayDirectories;
    QStringList m_dvdDirectories;

    QElapsedTimer m_reloadTimer;

    int m_movieSum = 0;

    bool m_running = false;
    bool m_aborted = false;
};

} // namespace mediaelch
