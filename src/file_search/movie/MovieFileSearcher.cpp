#include "MovieFileSearcher.h"

#include "database/MoviePersistence.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"
#include "log/Log.h"
#include "src/file_search/movie/MovieDirectorySearcher.h"

#include <QApplication>
#include <QDirIterator>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QtConcurrent>

namespace mediaelch {

MovieFileSearcher::MovieFileSearcher(QObject* parent) : QObject(parent), m_store{new MovieLoaderStore(this)}
{
    connect(this, &MovieFileSearcher::started, this, [this]() { m_reloadTimer.start(); });
    connect(this, &MovieFileSearcher::finished, this, [this]() {
        qCDebug(c_movie) << "[Movies] Reloading took" << m_reloadTimer.elapsed() << "ms";
        m_reloadTimer.invalidate();
    });
}

MovieFileSearcher::~MovieFileSearcher()
{
    // FIXME: For CLI, we must ensure that no thread is still running!
    m_aborted = true;
}

void MovieFileSearcher::setMovieDirectories(const QVector<mediaelch::MediaDirectory>& directories)
{
    abort(true);
    const auto& filter = Settings::instance()->advanced()->movieFilters();
    m_directories.clear();
    for (const auto& dir : directories) {
        if (filter.isFolderExcluded(dir.path.dirName())) {
            qCWarning(c_movie) << "[Movies] Movie directory is excluded by advanced settings:" << dir.path;
            continue;
        }

        if (!dir.path.isReadable()) {
            qCDebug(c_movie) << "[Movies] Movie directory is not readable, skipping:" << dir.path.path();
            continue;
        }

        m_directories.append(dir);
    }
}

void MovieFileSearcher::reload(bool reloadFromDisk)
{
    if (m_running) {
        qCCritical(c_movie) << "[Movies] Search already in progress";
        return;
    }

    qCInfo(c_movie) << "[Movies] Start reloading movies from" << (reloadFromDisk ? "disk" : "cache");

    m_aborted = false;
    m_running = true;

    emit started();
    emit statusChanged(tr("Searching for Movies..."));

    // Each call to processEvents() in a slot could potentially have triggered abort()
    if (m_aborted) {
        return;
    }

    MoviePersistence persistence{*Manager::instance()->database()};
    if (reloadFromDisk) {
        persistence.clearAllMovies();
    }

    Manager::instance()->movieModel()->clear();

    emit percentChanged(0.f, Constants::MovieFileSearcherProgressMessageId);

    // Each call to processEvents() in a slot could potentially have triggered abort()
    if (m_aborted) {
        return;
    }

    for (mediaelch::MediaDirectory movieDir : asConst(m_directories)) {
        // If we auto-reload directories, but didn't force to load _all_ movies, then we need
        // to clear all movies of that directory.  If reloadFromDisk is set, then the database
        // was cleared above.  If the directory is disabled, we also clear the cache if
        // autoReload is on.
        if (movieDir.autoReload && !reloadFromDisk) {
            persistence.clearMoviesInDirectory(movieDir.path);
        }
        if (!movieDir.disabled) {
            movieDir.autoReload = movieDir.autoReload || reloadFromDisk;
            m_directoryQueue.enqueue(std::move(movieDir));
        }
    }

    loadNext();
}

void MovieFileSearcher::onDirectoryLoaded(MovieLoader* job)
{
    // deleteLater() must not be called directly! it lives in other thread with its own event queue.
    // TODO: Refactor thread setup; don't pass object that lives on another thread?
    auto dls = makeDeleteLaterScope(job);
    // There is always only one job.  However, due to queued connections and the MovieLoader
    // being in another thread's event loop, it may happen that this slot is being
    // called with an old MovieLoader object.  If that is the case, just ignore it.
    // From the Qt docs:
    // > Warning: Deleting a QObject while pending events are waiting to be delivered can cause
    // > a crash. You must not delete the QObject directly if it exists in a different thread
    // > than the one currently executing. Use deleteLater() instead, which will cause the event
    // > loop to delete the object after all pending events have been delivered to it.
    if (job != m_currentJob) {
        return;
    }
    m_currentJob = nullptr;

    if (m_aborted || job->isAborted()) {
        // To avoid changes to the model, _after_ the users aborts, don't add any
        // movies to the model.
        m_store->clear();

    } else {
        // Note: This file searcher is the parent of all movies, but the model
        //       handles them.
        Manager::instance()->movieModel()->addMovies(m_store->takeAll(this));
        loadNext();
    }
}

void MovieFileSearcher::onPercentChange(worker::Job* job, float percent)
{
    Q_UNUSED(job)
    emit percentChanged(percent, Constants::MovieFileSearcherProgressMessageId);
}

void MovieFileSearcher::onProgressText(MovieLoader* job, QString text)
{
    Q_UNUSED(job)
    emit progressText(text);
}

void MovieFileSearcher::loadNext()
{
    if (m_aborted) {
        // no signal because aborted
        return;
    }

    MediaElch_Assert(m_running);

    if (m_directoryQueue.isEmpty()) {
        emit finished();
        return;
    }

    MediaElch_Assert(m_store != nullptr);

    mediaelch::MediaDirectory dir = m_directoryQueue.dequeue();

    QString currentStatus = tr("Searching for movies...");
    const auto active = std::count_if(m_directories.cbegin(),
        m_directories.cend(), //
        [](const mediaelch::MediaDirectory& d) { return !d.disabled; });

    if (active > 1) {
        const auto finished = active - m_directoryQueue.size();
        currentStatus += QStringLiteral(" (%1/%2)").arg(QString::number(finished), QString::number(active));
    }

    emit statusChanged(currentStatus);
    if (m_aborted) {
        return;
    }

    MovieLoader* loader = nullptr;
    if (dir.autoReload) {
        loader = new MovieDiskLoader(dir, *m_store, Settings::instance()->advanced()->movieFilters(), nullptr);
    } else {
        loader = new MovieDatabaseLoader(dir, *m_store, nullptr);
    }

    QThread* thread = mediaelch::createAutoDeleteThreadWithMovieLoader(loader, this);
    connect(loader, &MovieLoader::loaderFinished, this, &MovieFileSearcher::onDirectoryLoaded, Qt::QueuedConnection);
    connect(loader, &MovieLoader::percentChanged, this, &MovieFileSearcher::onPercentChange, Qt::QueuedConnection);
    connect(loader, &MovieLoader::progressText, this, &MovieFileSearcher::onProgressText, Qt::QueuedConnection);

    MediaElch_Assert(m_currentJob == nullptr);
    m_currentJob = loader;
    thread->start(QThread::HighPriority);
}

void MovieFileSearcher::abort(bool quiet)
{
    if (!quiet) {
        qCDebug(generic) << "[Movie] Aborted movie file searcher!";
    }
    m_aborted = true;
    m_running = false;
    m_directoryQueue.clear();

    if (m_currentJob != nullptr) {
        MediaElch_Assert(m_currentJob->kill());
        m_currentJob = nullptr;
    }
    m_store->clear();
}

} // namespace mediaelch
