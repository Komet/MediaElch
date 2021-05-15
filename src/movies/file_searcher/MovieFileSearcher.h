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

public slots:
    void reload(bool force);
    void abort();

signals:
    void searchStarted(QString);
    void progress(int current, int max, int messageBarId);
    void moviesLoaded();
    void currentDir(QString);

private slots:
    void onDirectoryLoaded(MovieDirectorySearcher* searcher);
    void onDirectoryStartsLoading(int approximateMovieCount);
    void onMovieProcessed(Movie* movie);

private:
    /// \brief Resets all counters, internal variables and so on.
    void resetInternalState();

private:
    QVector<SettingsDir> m_directories;
    QVector<MovieDirectorySearcher*> m_searchers;
    QElapsedTimer m_reloadTimer;

    int m_approxMovieSum = 0;
    int m_moviesProcessed = 0;

    bool m_running = false;
    bool m_aborted = false;
};

} // namespace mediaelch
