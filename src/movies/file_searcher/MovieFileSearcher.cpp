#include "MovieFileSearcher.h"

#include "MovieDirectorySearcher.h"
#include "data/Subtitle.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"
#include "log/Log.h"

#include <QApplication>
#include <QDirIterator>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QtConcurrent>

namespace mediaelch {

MovieFileSearcher::MovieFileSearcher(QObject* parent) : QObject(parent), m_aborted{false}
{
    connect(this, &MovieFileSearcher::searchStarted, this, [this]() { m_reloadTimer.start(); });
    connect(this, &MovieFileSearcher::moviesLoaded, this, [this]() {
        qCDebug(generic) << "[MovieFileSearcher] Reloading took" << m_reloadTimer.elapsed() << "ms";
        m_reloadTimer.invalidate();
    });
}

void MovieFileSearcher::setMovieDirectories(const QVector<SettingsDir>& directories)
{
    abort();
    m_directories.clear();
    for (const auto& dir : directories) {
        if (Settings::instance()->advanced()->isFolderExcluded(dir.path.dirName())) {
            qCWarning(generic) << "[MovieFileSearcher] Movie directory is excluded by advanced settings:" << dir.path;
            continue;
        }

        if (!dir.path.isReadable()) {
            qCDebug(generic) << "[MovieFileSearcher] Movie directory is not readable, skipping:" << dir.path.path();
            continue;
        }

        qCDebug(generic) << "[MovieFileSearcher] Adding movie directory" << dir.path.path();
        m_directories.append(dir);
    }
}

void MovieFileSearcher::reload(bool force)
{
    if (m_running) {
        qCCritical(generic) << "[MovieFileSearcher] Search already in progress";
        return;
    }

    qInfo() << "[MovieFileSearcher] Start reloading; Forced=" << force;
    emit searchStarted(tr("Searching for Movies..."));

    resetInternalState();
    m_running = true;

    if (force) {
        Manager::instance()->database()->clearAllMovies();
    }

    Manager::instance()->movieModel()->clear();

    // Each call to processEvents() could potentially have triggered abort()
    emit progress(0, 0, Constants::MovieFileSearcherProgressMessageId);
    if (m_aborted) {
        return;
    }

    // Create searchers...
    for (const SettingsDir& movieDir : asConst(m_directories)) {
        if (movieDir.disabled) {
            continue;
        }

        if (movieDir.autoReload || force) {
            // We need to reload from disk...
            auto* searcher = new MovieDirectorySearcher(movieDir, movieDir.separateFolders, this);
            m_searchers.push_back(searcher);
            connect(searcher, &MovieDirectorySearcher::loaded, this, &MovieFileSearcher::onDirectoryLoaded);
            connect(searcher, &MovieDirectorySearcher::movieProcessed, this, &MovieFileSearcher::onMovieProcessed);
            connect(
                searcher, &MovieDirectorySearcher::startLoading, this, &MovieFileSearcher::onDirectoryStartsLoading);

        } else {
            // ...or from the cached database.
            // Note: We do this in a blocking way for now.
            // TODO: Move into worker.
            const QVector<Movie*> moviesFromDb =
                Manager::instance()->database()->moviesInDirectory(mediaelch::DirectoryPath(movieDir.path));
            if (moviesFromDb.count() > 0) {
                QtConcurrent::blockingMap(moviesFromDb, [](Movie* movie) {
                    movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), false, false);
                });
                Manager::instance()->movieModel()->addMovies(moviesFromDb);
            }
        }
    }

    if (m_searchers.isEmpty()) {
        emit moviesLoaded();
        return;
    }

    // ...then start
    for (auto* searcher : asConst(m_searchers)) {
        Manager::instance()->database()->clearMoviesInDirectory(mediaelch::DirectoryPath(searcher->directory().path));
        searcher->load();
    }
}

void MovieFileSearcher::onDirectoryLoaded(MovieDirectorySearcher* searcher)
{
    if (m_aborted) {
        return;
    }

    QVector<Movie*> movies = searcher->movies();
    const mediaelch::DirectoryPath dir(searcher->directory().path);

    qCDebug(generic) << "[MovieFileSearcher] Directory loaded:" << dir.toNativePathString();

    if (m_searchers.size() <= 1) {
        // Reset the progress bar and show a "uncertain" bar without progress.
        // Note: It could happen that this isn't called at all if a second file
        //       searcher goes into this function during the QApplication::processEvents()
        //       below.  But if we remove the searcher here, we may emit moviesLoaded() before
        //       this slot has completed.
        emit progress(0, 0, Constants::MovieFileSearcherProgressMessageId);
        if (m_aborted) {
            return;
        }
    }

    // This code looks ugly but does essentially this:
    // Chunk the vector so that N movies are committed into the database.
    // This avoid adding thousands of movies at once.
    // TODO: Do in another thread.
    Manager::instance()->database()->transaction();
    for (int i = 0; i < movies.size(); ++i) {
        if (i % 40 == 0 && i > 0) {
            // Commit previous transaction and begin new one
            Manager::instance()->database()->commit();
            Manager::instance()->database()->transaction();
        }

        Movie* movie = movies.at(i);
        // Parent is always the MovieFileSearcher and _not_ the MovieModel.
        movie->setParent(this);
        // Note: We can't do it in MovieDirectorySearcher, because we have to use the database connection's thread.
        movie->setLabel(Manager::instance()->database()->getLabel(movie->files()));
        Manager::instance()->database()->addMovie(movie, dir);

        if (i % 40 == 0 && i > 0) {
            // Each call to processEvents() could potentially have triggered abort();
            QApplication::processEvents();
            if (m_aborted) {
                return;
            }
        }
    }
    Manager::instance()->database()->commit();
    Manager::instance()->movieModel()->addMovies(movies);

    m_searchers.removeOne(searcher);
    const bool isFinished = m_searchers.isEmpty();

    searcher->deleteLater();
    if (isFinished) {
        emit moviesLoaded();
    }
}

void MovieFileSearcher::onDirectoryStartsLoading(int approximateMovieCount)
{
    m_approxMovieSum += approximateMovieCount;
}

void MovieFileSearcher::onMovieProcessed(Movie* movie)
{
    ++m_moviesProcessed;
    movie->setParent(this);

    if (m_approxMovieSum > 0 && m_moviesProcessed <= m_approxMovieSum) {
        emit progress(m_moviesProcessed, m_approxMovieSum, Constants::MovieFileSearcherProgressMessageId);
    }

    // Reduce the noise a bit. "20" felt like a nice value.
    if (m_moviesProcessed % 20 == 0) {
        emit currentDir(movie->name());
    }
}

void MovieFileSearcher::resetInternalState()
{
    m_moviesProcessed = 0;
    m_approxMovieSum = 0;

    m_aborted = false;
    m_running = false;
}

void MovieFileSearcher::abort()
{
    m_aborted = true;
    m_running = false;

    for (MovieDirectorySearcher* searcher : asConst(m_searchers)) {
        searcher->abort();
        searcher->deleteLater();
    }
    m_searchers.clear();
}

} // namespace mediaelch
