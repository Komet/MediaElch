#include "MovieDirectorySearcher.h"

#include "database/Database.h"
#include "database/MoviePersistence.h"
#include "globals/Manager.h"
#include "globals/MediaDirectory.h"
#include "log/Log.h"
#include "media/FilenameUtils.h"
#include "settings/Settings.h"


#include <QMutexLocker>
#include <QtConcurrent>
#include <memory>

namespace mediaelch {

MovieLoader::MovieLoader(MovieLoaderStore* store, QObject* parent) : worker::Job(parent), m_store{store}
{
    // Note: Because instances of this class are run in another thread with
    //       another event loop, we can't use auto-delete or the object would be
    //       deleted before the slots are invoked (which are enqueued because of
    //       different threads).
    setAutoDelete(false);
    // Convenience signal
    connect(this, &worker::Job::finished, this, [this](worker::Job* /*unused*/) { emit loaderFinished(this); });
}

void MovieLoaderStore::addMovie(Movie* movie)
{
    movie->setParent(nullptr);
    movie->moveToThread(thread());
    movie->setParent(this);

    QMutexLocker locker(&m_lock);
    m_movies.append(movie);
}

void MovieLoaderStore::addMovies(const QVector<Movie*>& movies)
{
    for (Movie* movie : movies) {
        movie->setParent(nullptr);
        movie->moveToThread(thread());
        movie->setParent(this);
    }

    QMutexLocker locker(&m_lock);
    m_movies.append(movies);
}

QVector<Movie*> MovieLoaderStore::takeAll(QObject* parent)
{
    QMutexLocker locker(&m_lock);
    QVector<Movie*> movies = std::move(m_movies);
    m_movies = {};
    locker.unlock();

    for (Movie* movie : asConst(movies)) {
        movie->setParent(parent);
    }
    return movies;
}

void MovieLoaderStore::clear()
{
    QMutexLocker locker(&m_lock);
    qDeleteAll(m_movies);
    m_movies.clear();
}

MovieDiskLoader::MovieDiskLoader(mediaelch::MediaDirectory dir,
    MovieLoaderStore& store,
    FileFilter filter,
    QObject* parent) :
    MovieLoader(&store, parent), m_dir{std::move(dir)}, m_filter{std::move(filter)}, m_db{Database::newConnection(this)}
{
}

MovieDiskLoader::~MovieDiskLoader()
{
    qDeleteAll(m_movies);
    m_movies.clear();
    delete m_db;
}

void MovieDiskLoader::doStart()
{
    qCInfo(c_movie) << "[Movie] Scanning directory:" << QDir::toNativeSeparators(m_dir.path.path());

    // No filter, no media files...
    if (!m_filter.hasValidFilters()) {
        qCCritical(c_movie) << "[Movie] Can't scan for movies because there is no valid movie file filter!";
        if (!isAborted()) {
            emitFinished();
        }
        return;
    }

    emitPercent(0, 0);
    emit progressText(this, "");
    if (isAborted()) {
        return;
    }

    loadMovieContents();

    if (isAborted()) {
        return;
    }

    m_processed = 0;
    m_approxTotal = m_dir.separateFolders ? m_contents.size() : 0;
    emitPercent(m_processed, m_approxTotal);

    qCDebug(c_movie) << "[Movie] Creating movies for directory:" << QDir::toNativeSeparators(m_dir.path.path());

    // Can be blocking as this class should NOT be run in the GUI thread and
    // emitting signals is thread safe.
    QtConcurrent::blockingMap(m_contents, [this](const QStringList& files) { createMovie(files); });

    storeAndAddToDatabase();

    if (!isAborted()) {
        emitFinished();
    }
}

bool MovieDiskLoader::doKill()
{
    m_aborted.store(true);
    return true;
}

void MovieDiskLoader::loadMovieContents()
{
    QQueue<QString> dirs;
    dirs.enqueue(m_dir.path.path());

    QString lastDir;

    while (!dirs.isEmpty()) {
        QString dir(dirs.dequeue());

        QDirIterator it(dir, m_filter.fileGlob, QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);

        while (it.hasNext()) {
            if (isAborted()) {
                return;
            }
            it.next();

            QString dirName = it.fileInfo().dir().dirName();
            QString fileName = it.fileName(); // may actually be a directory name

            const bool isFile = it.fileInfo().isFile();
            const bool isDir = it.fileInfo().isDir();
            bool isSpecialDir = false; // set to true for DVD or BluRay Structure

            if (isFile && m_filter.isFileExcluded(fileName)) {
                continue;
            }

            if ((isDir && m_filter.isFolderExcluded(fileName)) || m_filter.isFolderExcluded(dirName)) {
                continue;
            }

            // Skips Extras files
            if (isFile
                && (fileName.contains("-trailer", Qt::CaseInsensitive)
                    || fileName.contains("-sample", Qt::CaseInsensitive)
                    || fileName.contains("-behindthescenes", Qt::CaseInsensitive)
                    || fileName.contains("-deleted", Qt::CaseInsensitive)
                    || fileName.contains("-featurette", Qt::CaseInsensitive)
                    || fileName.contains("-interview", Qt::CaseInsensitive)
                    || fileName.contains("-scene", Qt::CaseInsensitive)
                    || fileName.contains("-short", Qt::CaseInsensitive))) {
                continue;
            }

            // Skip folders and all files inside them
            if (isDir
                && (QString::compare(".actors", fileName, Qt::CaseInsensitive) == 0
                    || QString::compare("extras", fileName, Qt::CaseInsensitive) == 0
                    || QString::compare("featurettes", fileName, Qt::CaseInsensitive) == 0
                    || QString::compare("extrafanart", fileName, Qt::CaseInsensitive) == 0
                    || QString::compare("extrathumbs", fileName, Qt::CaseInsensitive) == 0)) {
                continue;
            }

            // Skip BluRay backup folder
            if (QString::compare("backup", dirName, Qt::CaseInsensitive) == 0
                && QString::compare("index.bdmv", fileName, Qt::CaseInsensitive) == 0) {
                continue;
            }

            if (isFile && QString::compare("index.bdmv", fileName, Qt::CaseInsensitive) == 0) {
                QDir bluRayDir(it.fileInfo().dir());
                if (QString::compare(bluRayDir.dirName(), "BDMV", Qt::CaseInsensitive) == 0) {
                    bluRayDir.cdUp();
                }
                m_bluRayDirectories << bluRayDir.path();
                isSpecialDir = true;
            }
            if (QString::compare("VIDEO_TS.IFO", fileName, Qt::CaseInsensitive) == 0) {
                QDir videoDir(it.fileInfo().dir());
                if (QString::compare(videoDir.dirName(), "VIDEO_TS", Qt::CaseInsensitive) == 0) {
                    videoDir.cdUp();
                }
                m_dvdDirectories << videoDir.path();
                isSpecialDir = true;
            }

            const QString dirPath = it.fileInfo().path();
            if (isFile || isSpecialDir) {
                if (!m_contents.contains(dirPath)) {
                    m_contents.insert(dirPath, {});
                }
                m_contents[dirPath].append(it.filePath());
                m_lastModifications.insert(it.filePath(), it.fileInfo().lastModified());
            } else {
                dirs.enqueue(it.filePath());
            }

            if (dirName != lastDir) {
                lastDir = dirName;
                // TODO: Use SignalThrottler
                if (m_contents.count() % 40 == 0) {
                    emit progressText(this, dirName);
                }
            }
        }
    }
}

void MovieDiskLoader::createMovie(QStringList files)
{
    // Note: This method is called in parallel!

    DiscType discType = DiscType::Single;

    // BluRay handling
    for (const QString& path : asConst(m_bluRayDirectories)) {
        if (!files.isEmpty() && (files.first().startsWith(path + "/") || files.first().startsWith(path + "\\"))) {
            QStringList f;
            for (const QString& file : files) {
                if (file.endsWith("index.bdmv", Qt::CaseInsensitive)) {
                    f.append(file);
                }
            }
            files = f;
            discType = DiscType::BluRay;
        }
    }

    // DVD handling
    for (const QString& path : asConst(m_dvdDirectories)) {
        if (!files.isEmpty() && (files.first().startsWith(path + "/") || files.first().startsWith(path + "\\"))) {
            QStringList f;
            for (const QString& file : asConst(files)) {
                if (file.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
                    f.append(file);
                }
            }
            files = f;
            discType = DiscType::Dvd;
        }
    }

    if (files.isEmpty()) {
        return;
    }

    if (files.count() == 1 || m_dir.separateFolders) {
        // single file or in separate folder
        mediaelch::file::sortFilenameList(files);
        auto* movie = new Movie(files, nullptr);
        movie->setInSeparateFolder(m_dir.separateFolders);
        movie->setFileLastModified(m_lastModifications.value(files.at(0)));
        movie->setDiscType(discType);

        // Note: "Label" is set in storeAndAddToDatabase()

        movie->setChanged(false);
        movie->controller()->loadData(Manager::instance()->mediaCenterInterface());
        if (discType == DiscType::Single) {
            QFileInfo mFi(files.first());

            // Get subtitle filters from AdvancedSettings
            const mediaelch::FileFilter& subtitleFilters = Settings::instance()->advanced()->subtitleFilters();

            // Build list of subtitle extensions from the filters
            QStringList subtitleExtensions;
            for (const QString& filter : subtitleFilters.fileGlob) {
                if (filter.startsWith("*.")) {
                    subtitleExtensions << filter.mid(2); // Remove "*."
                }
            }

            // Create a filter string for QDir
            QStringList subtitleGlobs;
            for (const QString& ext : subtitleExtensions) {
                subtitleGlobs << mFi.completeBaseName() + "*." + ext;
            }

            const QList<QFileInfo> subFiles =
                mFi.dir().entryInfoList(subtitleGlobs, QDir::Files | QDir::NoDotAndDotDot);

            for (const QFileInfo& subFi : subFiles) {
                QString subFileName = subFi.fileName().mid(mFi.completeBaseName().length() + 1);
                QStringList parts = subFileName.split(QRegularExpression(R"(\s+|\-+|\.+)"));
                if (parts.isEmpty()) {
                    continue;
                }
                parts.takeLast();

                QStringList subSubFiles = QStringList() << subFi.fileName();
                // Special handling for .sub/.idx pairs
                if (QString::compare(subFi.suffix(), "sub", Qt::CaseInsensitive) == 0) {
                    QFileInfo subIdxFi(subFi.absolutePath() + "/" + subFi.completeBaseName() + ".idx");
                    if (subIdxFi.exists()) {
                        subSubFiles << subIdxFi.fileName();
                    }
                }
                auto* subtitle = new Subtitle(movie);
                subtitle->setFiles(subSubFiles);
                if (parts.contains("forced", Qt::CaseInsensitive)) {
                    subtitle->setForced(true);
                    parts.removeAll("forced");
                }
                if (!parts.isEmpty()) {
                    subtitle->setLanguage(parts.first());
                }
                subtitle->setChanged(false);
                movie->addSubtitle(subtitle, true);
            }
        }

        // As this method is called in parallel, we may be in another thread.
        movie->moveToThread(thread());

        QMutexLocker lock(&m_mutex);
        m_movies.append(movie);
        const auto size = m_movies.size();
        lock.unlock();

        emitPercent(++m_processed, m_approxTotal);
        if (size % 40 == 0) {
            // TODO: Use SignalThrottler
            emit progressText(this, movie->title());
        }

    } else {
        QMap<QString, QStringList> stacked;
        while (!files.isEmpty()) {
            QString file = files.takeLast();

            QString stackedBase = mediaelch::file::stackedBaseName(file);
            stacked.insert(stackedBase, {file});

            for (int fileIndex = 0; fileIndex < files.count();) {
                const QString& f = files[fileIndex];

                if (mediaelch::file::stackedBaseName(f) == stackedBase) {
                    stacked[stackedBase].append(f);
                    files.removeAt(fileIndex);
                } else {
                    fileIndex++;
                }
            }
        }
        QMapIterator<QString, QStringList> it(stacked);
        while (it.hasNext()) {
            it.next();
            if (it.value().isEmpty()) {
                continue;
            }
            QStringList stackedFiles = it.value();
            stackedFiles.sort();
            auto* movie = new Movie(stackedFiles, nullptr);
            movie->setInSeparateFolder(m_dir.separateFolders);
            movie->setFileLastModified(m_lastModifications.value(it.value().at(0)));
            movie->controller()->loadData(Manager::instance()->mediaCenterInterface());

            // As this method is called in parallel, we may be in another thread.
            movie->moveToThread(thread());

            QMutexLocker lock(&m_mutex);
            m_movies.append(movie);
            const auto size = m_movies.size();
            lock.unlock();

            emitPercent(++m_processed, m_approxTotal);
            if (size % 40 == 0) {
                // TODO: Use SignalThrottler
                emit progressText(this, movie->title());
            }
        }
    }
}

void MovieDiskLoader::storeAndAddToDatabase()
{
    if (isAborted()) {
        return;
    }

    emitPercent(0, 0);
    emit progressText(this, tr("Storing movies in database..."));

    mediaelch::MoviePersistence persistence{*m_db};
    m_db->db().transaction();
    for (Movie* movie : asConst(m_movies)) {
        // See also: Use https://stackoverflow.com/a/47473949/1603627
        // We do this in just one thread.
        movie->setLabel(m_db->getLabel(movie->files()));
        persistence.addMovie(movie, m_dir.path);
        m_store->addMovie(movie);
    }
    m_db->db().commit();
    m_movies.clear();
}

void MovieDatabaseLoader::doStart()
{
    qCInfo(c_movie) << "[Movie] Loading entries from database for directory:"
                    << QDir::toNativeSeparators(m_dir.path.path());

    emitPercent(0, 0);
    emit progressText(this, "");

    QVector<Movie*> movies;
    {
        std::unique_ptr<Database> db(Database::newConnection(this));
        mediaelch::MoviePersistence persistence{*db};
        movies = persistence.moviesInDirectory(m_dir.path, this);
    }
    if (isAborted()) {
        return;
    }
    if (movies.count() <= 0) {
        emitFinished();
        return;
    }

    // Note, this takes less than a few seconds. No need to check whether we're aborted or not.
    QtConcurrent::blockingMap(movies,
        [](Movie* movie) { //
            movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), false, false);
        });

    if (isAborted()) {
        return;
    }

    emitPercent(1, 1);
    emit progressText(this, "");

    m_store->addMovies(movies);

    if (!isAborted()) {
        emitFinished();
    }
}

bool MovieDatabaseLoader::doKill()
{
    m_aborted.store(true);
    return true;
}

QThread* createAutoDeleteThreadWithMovieLoader(MovieLoader* worker, QObject* threadParent)
{
    QThread* thread = new QThread(threadParent);
    MediaElch_Assert(thread != nullptr);
    thread->setObjectName("movie-loader-thread");
    worker->moveToThread(thread);

    // Startup & delete setup
    QObject::connect(thread, &QThread::started, worker, &MovieLoader::start);
    QObject::connect(worker, &MovieLoader::destroyed, thread, &QThread::quit);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    return thread;
}

} // namespace mediaelch
