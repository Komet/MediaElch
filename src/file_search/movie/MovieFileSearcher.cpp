#include "MovieFileSearcher.h"

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

void MovieFileSearcher::setMovieDirectories(const QVector<mediaelch::MediaDirectory>& directories)
{
    abort(true);

    m_directories.clear();
    for (const auto& dir : directories) {
        if (Settings::instance()->advanced()->isFolderExcluded(dir.path.dirName())) {
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

    if (reloadFromDisk) {
        Manager::instance()->database()->clearAllMovies();
    }

    Manager::instance()->movieModel()->clear();

    emit percentChanged(0.f, Constants::MovieFileSearcherProgressMessageId);

    // Each call to processEvents() in a slot could potentially have triggered abort()
    if (m_aborted) {
        return;
    }

    for (mediaelch::MediaDirectory movieDir : asConst(m_directories)) {
        if (!movieDir.disabled) {
            movieDir.autoReload = movieDir.autoReload || reloadFromDisk;
            m_directoryQueue.enqueue(std::move(movieDir));
        }
    }

    loadNext();
}

void MovieFileSearcher::onDirectoryLoaded(MovieLoader* job)
{
    // There is always only one job. Ensure that we don't mix up anything.
    Q_ASSERT(job == m_currentJob);
    m_currentJob = nullptr;
    job->deleteLater();

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
    connect(loader, &MovieLoader::loaderFinished, this, &MovieFileSearcher::onDirectoryLoaded);
    connect(loader, &MovieLoader::percentChanged, this, &MovieFileSearcher::onPercentChange);
    connect(loader, &MovieLoader::progressText, this, &MovieFileSearcher::onProgressText);

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
