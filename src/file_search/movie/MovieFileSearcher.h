#pragma once

#include "globals/MediaDirectory.h"

#include <QDir>
#include <QElapsedTimer>
#include <QHash>
#include <QObject>
#include <QQueue>
#include <QTime>
#include <QVector>
#include <memory>

namespace mediaelch {

namespace worker {
class Job;
}

class MovieLoader;
class MovieLoaderStore;

/// \brief Class responsible for (re-)loading all movies inside given directories.
///
/// \par Example
/// \code{cpp}
///   MovieFileSearcher searcher;
///   searcher.setMovieDirectories(directories);
///   searcher.reload(true);
/// \endcode
class MovieFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit MovieFileSearcher(QObject* parent = nullptr);
    ~MovieFileSearcher() override;

    /// \brief Sets the directories to scan for movies. Not readable directories are skipped.
    void setMovieDirectories(const QVector<mediaelch::MediaDirectory>& directories);

public slots:
    /// Reload movies. Emits finished() when reloaded.
    void reload(bool reloadFromDisk);
    void abort(bool quiet = false);

signals:
    void started();
    void statusChanged(QString userText);
    void percentChanged(float percent, int messageBarId);
    /// \brief Text representing the current status, e.g. the current directory.
    void progressText(QString text);

    void finished();

private slots:
    void onDirectoryLoaded(MovieLoader* job);
    void onPercentChange(worker::Job* job, float percent);
    void onProgressText(MovieLoader* job, QString text);

private:
    void loadNext();

private:
    QVector<mediaelch::MediaDirectory> m_directories;
    QElapsedTimer m_reloadTimer;

    /// \brief Directories that need to be scanned.
    QQueue<mediaelch::MediaDirectory> m_directoryQueue;
    MovieLoaderStore* m_store = nullptr;
    MovieLoader* m_currentJob = nullptr;

    bool m_running = false;
    bool m_aborted = false;
};

} // namespace mediaelch
