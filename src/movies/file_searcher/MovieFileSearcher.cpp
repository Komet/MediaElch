#include "MovieFileSearcher.h"

#include "MovieDirectorySearcher.h"
#include "data/Subtitle.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"

#include <QApplication>
#include <QDebug>
#include <QDirIterator>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QtConcurrent>

namespace mediaelch {

MovieFileSearcher::MovieFileSearcher(QObject* parent) : QObject(parent), m_aborted{false}
{
    connect(this, &MovieFileSearcher::searchStarted, this, [this]() { m_reloadTimer.start(); });
    connect(this, &MovieFileSearcher::moviesLoaded, this, [this]() {
        qDebug() << "[MovieFileSearcher] Reloading took" << m_reloadTimer.elapsed() << "ms";
        m_reloadTimer.invalidate();
    });
}

void MovieFileSearcher::loadMovieData(Movie* movie)
{
    movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), false, false);
}

void MovieFileSearcher::setMovieDirectories(const QVector<SettingsDir>& directories)
{
    abort();
    m_directories.clear();
    for (const auto& dir : directories) {
        if (Settings::instance()->advanced()->isFolderExcluded(dir.path.dirName())) {
            qWarning() << "[MovieFileSearcher] Movie directory is excluded by advanced settings:" << dir.path;
            continue;
        }

        if (!dir.path.isReadable()) {
            qDebug() << "[MovieFileSearcher] Movie directory is not readable, skipping:" << dir.path.path();
            continue;
        }

        qDebug() << "[MovieFileSearcher] Adding movie directory" << dir.path.path();
        m_directories.append(dir);
    }
}

void MovieFileSearcher::reload(bool force)
{
    if (m_running) {
        qCritical() << "[MovieFileSearcher] Search already in progress";
        return;
    }

    resetInternalState();

    qInfo() << "[MovieFileSearcher] Start reloading; Forced=" << force;
    emit searchStarted(tr("Searching for Movies..."));


    m_aborted = false;
    m_running = true;

    QApplication::processEvents();

    if (force) {
        Manager::instance()->database()->clearAllMovies();
    }
    QApplication::processEvents();
    Manager::instance()->movieModel()->clear();
    QApplication::processEvents();

    emit progress(0, 0, Constants::MovieFileSearcherProgressMessageId);


    // Create searchers...
    for (const SettingsDir& movieDir : asConst(m_directories)) {
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
            const QVector<Movie*> moviesFromDb = Manager::instance()->database()->moviesInDirectory(movieDir.path);
            if (moviesFromDb.count() > 0) {
                QtConcurrent::blockingMap(moviesFromDb, MovieFileSearcher::loadMovieData);
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
        searcher->load();
    }
}

void MovieFileSearcher::onDirectoryLoaded(MovieDirectorySearcher* searcher)
{
    ++m_directoriesProcessed;
    qDebug() << "[MovieFileSearcher] Directory loaded:" << QDir::toNativeSeparators(searcher->directory().path.path());

    if (m_aborted) {
        return;
    }

    if (m_directoriesProcessed >= m_searchers.size()) {
        // Reset the progress bar.
        emit progress(0, 0, Constants::MovieFileSearcherProgressMessageId);
    }

    // This code looks ugly but does essentially this:
    // Chunk the vector so that N movies are committed into the database.
    // This avoid adding thousands of movies at once.
    // TODO: Do in another thread.
    QVector<Movie*> movies = searcher->movies();
    Manager::instance()->database()->transaction();
    for (int i = 0; i < movies.size(); ++i) {
        if (i % 40 == 0 && i > 0) {
            // Commit previous transaction and begin new one
            Manager::instance()->database()->commit();
            Manager::instance()->database()->transaction();
        }

        Movie* movie = movies.at(i);
        Manager::instance()->database()->add(movie, searcher->directory().path);

        if (i % 40 == 0 && i > 0) {
            QApplication::processEvents();
        }
    }
    Manager::instance()->database()->commit();
    Manager::instance()->movieModel()->addMovies(searcher->movies());

    if (!m_aborted && m_directoriesProcessed >= m_searchers.size()) {
        for (auto* searcher : asConst(m_searchers)) {
            searcher->deleteLater();
        }
        m_searchers.clear();

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
    m_reloadTimer.invalidate();

    m_directoriesProcessed = 0;
    m_moviesProcessed = 0;
    m_approxMovieSum = 0;

    m_aborted = false;
    m_running = false;
}


void MovieFileSearcher::scanDir(QString startPath,
    QString path,
    QVector<QStringList>& contents,
    bool separateFolders,
    bool firstScan)
{
    resetInternalState();
    m_aborted = false;
    emit currentDir(path.mid(startPath.length()));

    QDir dir(path);
    for (const QString& cDir : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (m_aborted) {
            return;
        }

        if (Settings::instance()->advanced()->isFolderExcluded(cDir)) {
            continue;
        }

        // Skip "Extras" folder
        if (QString::compare(cDir, "Extras", Qt::CaseInsensitive) == 0
            || QString::compare(cDir, ".actors", Qt::CaseInsensitive) == 0
            || QString::compare(cDir, ".AppleDouble", Qt::CaseInsensitive) == 0
            || QString::compare(cDir, "extrafanarts", Qt::CaseInsensitive) == 0) {
            continue;
        }

        // Handle DVD
        if (helper::isDvd(path + QDir::separator() + cDir)) {
            contents.append(QStringList() << QDir::toNativeSeparators(path + "/" + cDir + "/VIDEO_TS/VIDEO_TS.IFO"));
            continue;
        }

        // Handle BluRay
        if (helper::isBluRay(path + QDir::separator() + cDir)) {
            contents.append(QStringList() << QDir::toNativeSeparators(path + "/" + cDir + "/BDMV/index.bdmv"));
            continue;
        }

        // Don't scan subfolders when separate folders is checked
        if (!separateFolders || firstScan) {
            scanDir(startPath, path + "/" + cDir, contents, separateFolders);
        }
    }

    QStringList files;
    const QStringList entries = getFiles(path);
    for (const QString& file : entries) {
        if (m_aborted) {
            return;
        }

        if (Settings::instance()->advanced()->isFileExcluded(file)) {
            continue;
        }

        // Skip Extras files
        if (file.contains("-trailer", Qt::CaseInsensitive)            //
            || file.contains("-sample", Qt::CaseInsensitive)          //
            || file.contains("-behindthescenes", Qt::CaseInsensitive) //
            || file.contains("-deleted", Qt::CaseInsensitive)         //
            || file.contains("-featurette", Qt::CaseInsensitive)      //
            || file.contains("-interview", Qt::CaseInsensitive)       //
            || file.contains("-scene", Qt::CaseInsensitive)           //
            || file.contains("-short", Qt::CaseInsensitive)) {
            continue;
        }
        files.append(file);
    }
    files.sort();

    if (separateFolders) {
        QStringList movieFiles;
        for (const QString& file : files) {
            movieFiles.append(QDir::toNativeSeparators(path + "/" + file));
        }
        if (movieFiles.count() > 0) {
            contents.append(movieFiles);
        }
        return;
    }

    /* detect movies with multiple files*/
    QRegExp rx("([\\-_\\s\\.\\(\\)]+((a|b|c|d|e|f)|((part|cd|xvid)"
               "[\\-_\\s\\.\\(\\)]*\\d+))[\\-_\\s\\.\\(\\)]+)",
        Qt::CaseInsensitive);
    for (int i = 0, n = files.size(); i < n; i++) {
        if (m_aborted) {
            return;
        }

        QStringList movieFiles;
        QString file = files.at(i);
        if (file.isEmpty()) {
            continue;
        }

        movieFiles << QDir::toNativeSeparators(path + QDir::separator() + file);

        int pos = rx.lastIndexIn(file);
        if (pos != -1) {
            QString left = file.left(pos);
            QString right = file.mid(pos + rx.cap(0).size());
            for (int x = 0; x < n; x++) {
                QString subFile = files.at(x);
                if (subFile != file) {
                    if (subFile.startsWith(left) && subFile.endsWith(right)) {
                        movieFiles << QDir::toNativeSeparators(path + QDir::separator() + subFile);
                        files[x] = ""; // set an empty file name, this way we can skip this file in the main loop
                    }
                }
            }
        }
        if (movieFiles.count() > 0) {
            contents.append(movieFiles);
        }
    }
}

QStringList MovieFileSearcher::getFiles(QString path)
{
    const auto& filters = Settings::instance()->advanced()->movieFilters();
    QStringList files;

    for (const QString& file : filters.files(QDir(path))) {
        m_lastModifications.insert(
            QDir::toNativeSeparators(path + "/" + file), QFileInfo(path + QDir::separator() + file).lastModified());
        files.append(file);
    }
    return files;
}

void MovieFileSearcher::abort()
{
    for (auto* searcher : asConst(m_searchers)) {
        searcher->abort();
        searcher->deleteLater();
    }
    m_searchers.clear();
    m_aborted = true;
    m_running = false;
}

} // namespace mediaelch
