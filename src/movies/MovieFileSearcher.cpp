#include "MovieFileSearcher.h"

#include <QApplication>
#include <QDebug>
#include <QDirIterator>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentFilter>
#include <QtConcurrent/QtConcurrentMap>
#include <QtConcurrent/QtConcurrentRun>

#include "data/Subtitle.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"

MovieFileSearcher::MovieFileSearcher(QObject* parent) :
    QObject(parent), m_progressMessageId{Constants::MovieFileSearcherProgressMessageId}, m_aborted{false}
{
}

void MovieFileSearcher::reload(bool force)
{
    m_aborted = false;
    emit searchStarted(tr("Searching for Movies..."));

    if (force) {
        Manager::instance()->database()->clearMovies();
    }

    Manager::instance()->movieModel()->clear();
    m_lastModifications.clear();

    QVector<MovieContents> moviesContent;
    QVector<Movie*> dbMovies;
    QStringList bluRays;
    QStringList dvds;
    int movieSum = 0;

    emit progress(0, 0, m_progressMessageId);

    for (const auto& movieDir : m_directories) {
        if (m_aborted) {
            return;
        }
        movieSum += loadMoviesFromDirectory(movieDir, force, moviesContent, dbMovies, bluRays, dvds);
    }

    emit searchStarted(tr("Loading Movies..."));

    qDebug() << "Now processing files";
    int movieCounter = 0;
    QVector<Movie*> movies = loadAndStoreMoviesContents(moviesContent, bluRays, dvds, movieSum, movieCounter);
    if (m_aborted) {
        return;
    }
    emit currentDir("");

    QtConcurrent::blockingMapped(dbMovies, MovieFileSearcher::loadMovieData);

    for (Movie* movie : dbMovies) {
        if (m_aborted) {
            return;
        }
        movies.append(movie);
        if (movieCounter % 20 == 0) {
            emit currentDir(movie->name());
        }
        emit progress(++movieCounter, movieSum, m_progressMessageId);
    }

    for (Movie* movie : movies) {
        Manager::instance()->movieModel()->addMovie(movie);
    }

    if (!m_aborted) {
        emit moviesLoaded();
    }
}

Movie* MovieFileSearcher::loadMovieData(Movie* movie)
{
    movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), false, false);
    return movie;
}

/// Sets the directories to scan for movies. Not readable directories are skipped.
void MovieFileSearcher::setMovieDirectories(const QVector<SettingsDir>& directories)
{
    m_directories.clear();
    for (auto& dir : directories) {
        if (Settings::instance()->advanced()->isFolderExcluded(dir.path.dirName())) {
            qWarning() << "[MovieFileSearcher] Movie directory is excluded by advanced settings! "
                          "Is this intended? Directory:"
                       << dir.path.path();
            continue;
        }

        if (!dir.path.isReadable()) {
            qDebug() << "[MovieFileSearcher] Movie directory is not redable, skipping:" << dir.path.path();
            continue;
        }

        qDebug() << "[MovieFileSearcher] Adding movie directory" << dir.path.path();
        m_directories.append(dir);
    }
}

/**
 * @brief Scans the given path for movie files.
 * Results are in a list which contains a QStringList for every movie.
 * @param startPath Scanning started at this path
 * @param path Path to scan
 * @param contents List of contents
 * @param separateFolders Are concerts in separate folders
 * @param firstScan When this is true, subfolders are scanned, regardless of separateFolders
 * @deprecated
 */
Q_DECL_DEPRECATED void MovieFileSearcher::scanDir(QString startPath,
    QString path,
    QVector<QStringList>& contents,
    bool separateFolders,
    bool firstScan)
{
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
    QStringList entries = getFiles(path);
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

/// Get a list of files in a directory
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

int MovieFileSearcher::loadMoviesFromDirectory(const SettingsDir& movieDir,
    bool force,
    QVector<MovieContents>& moviesContent,
    QVector<Movie*>& dbMovies,
    QStringList& bluRays,
    QStringList& dvds)
{
    QString path = movieDir.path.path();
    int movieSum = 0;

    if (Settings::instance()->advanced()->isFolderExcluded(movieDir.path.dirName())) {
        qWarning() << "[MovieFileSearcher] Movie directory is excluded by advanced settings! "
                      "Is this intended? Directory:"
                   << movieDir.path.path();
        return movieSum;
    }

    QVector<Movie*> moviesFromDb;
    if (!movieDir.autoReload && !force) {
        moviesFromDb = Manager::instance()->database()->movies(path);
    }

    if (!movieDir.autoReload && !force && moviesFromDb.count() != 0) {
        dbMovies.append(moviesFromDb);
        return moviesFromDb.count();
    }

    emit currentDir(path);
    QApplication::processEvents();
    Manager::instance()->database()->clearMovies(path);
    QMap<QString, QStringList> contents;
    // No filter, no media files...
    if (!Settings::instance()->advanced()->movieFilters().hasFilter()) {
        return movieSum;
    }

    qDebug() << "Scanning directory: " << movieDir.path;
    QString lastDir;
    QDirIterator it(path,
        Settings::instance()->advanced()->movieFilters().filters(),
        QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files,
        QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    while (it.hasNext()) {
        if (m_aborted) {
            return movieSum;
        }
        it.next();

        QString dirName = it.fileInfo().dir().dirName();
        QString fileName = it.fileName();

        if (it.fileInfo().isFile() && Settings::instance()->advanced()->isFileExcluded(fileName)) {
            continue;
        }
        if (it.fileInfo().isDir() && Settings::instance()->advanced()->isFileExcluded(dirName)) {
            continue;
        }

        // Skips Extras files
        if (fileName.contains("-trailer", Qt::CaseInsensitive)            //
            || fileName.contains("-sample", Qt::CaseInsensitive)          //
            || fileName.contains("-behindthescenes", Qt::CaseInsensitive) //
            || fileName.contains("-deleted", Qt::CaseInsensitive)         //
            || fileName.contains("-featurette", Qt::CaseInsensitive)      //
            || fileName.contains("-interview", Qt::CaseInsensitive)       //
            || fileName.contains("-scene", Qt::CaseInsensitive)           //
            || fileName.contains("-short", Qt::CaseInsensitive)) {
            continue;
        }

        // Skip actors folder
        if (QString::compare(".actors", dirName, Qt::CaseInsensitive) == 0) {
            continue;
        }

        // Skip extras folder
        if (QString::compare("extras", dirName, Qt::CaseInsensitive) == 0) {
            continue;
        }

        // Skip extra fanarts folder
        if (QString::compare("extrafanart", dirName, Qt::CaseInsensitive) == 0) {
            continue;
        }

        // Skip extra thumbs folder
        if (QString::compare("extrathumbs", dirName, Qt::CaseInsensitive) == 0) {
            continue;
        }

        // Skip BluRay backup folder
        if (QString::compare("backup", dirName, Qt::CaseInsensitive) == 0
            && QString::compare("index.bdmv", fileName, Qt::CaseInsensitive) == 0) {
            continue;
        }

        if (dirName != lastDir) {
            lastDir = dirName;
            if (contents.count() % 20 == 0) {
                emit currentDir(dirName);
            }
        }

        if (QString::compare("index.bdmv", fileName, Qt::CaseInsensitive) == 0) {
            qDebug() << "Found BluRay structure";
            QDir bluRayDir(it.fileInfo().dir());
            if (QString::compare(bluRayDir.dirName(), "BDMV", Qt::CaseInsensitive) == 0) {
                bluRayDir.cdUp();
            }
            bluRays << bluRayDir.path();
        }
        if (QString::compare("VIDEO_TS.IFO", fileName, Qt::CaseInsensitive) == 0) {
            qDebug() << "Found DVD structure";
            QDir videoDir(it.fileInfo().dir());
            if (QString::compare(videoDir.dirName(), "VIDEO_TS", Qt::CaseInsensitive) == 0) {
                videoDir.cdUp();
            }
            dvds << videoDir.path();
        }

        QString filePath = it.fileInfo().path();
        if (!contents.contains(filePath)) {
            contents.insert(filePath, QStringList());
        }
        contents[filePath].append(it.filePath());
        m_lastModifications.insert(it.filePath(), it.fileInfo().lastModified());
    }
    MovieContents con;
    con.path = path;
    con.inSeparateFolder = movieDir.separateFolders;
    con.contents = contents;
    moviesContent.append(con);
    return contents.count();
}

QVector<Movie*> MovieFileSearcher::loadAndStoreMoviesContents(QVector<MovieFileSearcher::MovieContents>& moviesContent,
    QStringList& bluRays,
    QStringList& dvds,
    int& movieSum,
    int& movieCounter)
{
    QVector<Movie*> movies;
    for (const MovieContents& con : moviesContent) {
        Manager::instance()->database()->transaction();
        QMapIterator<QString, QStringList> itContents(con.contents);
        while (itContents.hasNext()) {
            if (m_aborted) {
                Manager::instance()->database()->commit();
                return movies;
            }
            itContents.next();
            QStringList files = itContents.value();

            DiscType discType = DiscType::Single;

            // BluRay handling
            for (const QString& path : bluRays) {
                if (!files.isEmpty()
                    && (files.first().startsWith(path + "/") || files.first().startsWith(path + "\\"))) {
                    QStringList f;
                    for (const QString& file : files) {
                        if (file.endsWith("index.bdmv", Qt::CaseInsensitive)) {
                            f.append(file);
                        }
                    }
                    files = f;
                    discType = DiscType::BluRay;
                    qDebug() << "It's a BluRay structure";
                }
            }

            // DVD handling
            for (const QString& path : dvds) {
                if (!files.isEmpty()
                    && (files.first().startsWith(path + "/") || files.first().startsWith(path + "\\"))) {
                    QStringList f;
                    for (const QString& file : files) {
                        if (file.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
                            f.append(file);
                        }
                    }
                    files = f;
                    discType = DiscType::Dvd;
                    qDebug() << "It's a DVD structure";
                }
            }

            if (files.isEmpty()) {
                continue;
            }

            if (files.count() == 1 || con.inSeparateFolder) {
                // single file or in separate folder
                files.sort();
                Movie* movie = new Movie(files, this);
                movie->setInSeparateFolder(con.inSeparateFolder);
                movie->setFileLastModified(m_lastModifications.value(files.at(0)));
                movie->setDiscType(discType);
                movie->setLabel(Manager::instance()->database()->getLabel(movie->files()));
                movie->setChanged(false);
                movie->controller()->loadData(Manager::instance()->mediaCenterInterface());
                if (discType == DiscType::Single) {
                    QFileInfo mFi(files.first());
                    for (const QFileInfo& subFi : mFi.dir().entryInfoList(
                             QStringList{"*.sub", "*.srt", "*.smi", "*.ssa"}, QDir::Files | QDir::NoDotAndDotDot)) {
                        QString subFileName = subFi.fileName().mid(mFi.completeBaseName().length() + 1);
                        QStringList parts = subFileName.split(QRegExp(R"(\s+|\-+|\.+)"));
                        if (parts.isEmpty()) {
                            continue;
                        }
                        parts.takeLast();

                        QStringList subFiles = QStringList() << subFi.fileName();
                        if (QString::compare(subFi.suffix(), "sub", Qt::CaseInsensitive) == 0) {
                            QFileInfo subIdxFi(subFi.absolutePath() + "/" + subFi.completeBaseName() + ".idx");
                            if (subIdxFi.exists()) {
                                subFiles << subIdxFi.fileName();
                            }
                        }
                        auto subtitle = new Subtitle(movie);
                        subtitle->setFiles(subFiles);
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
                Manager::instance()->database()->add(movie, con.path);
                movies.append(movie);
                // emit currentDir(movie->name());
            } else {
                QMap<QString, QStringList> stacked;
                while (!files.isEmpty()) {
                    QString file = files.takeLast();
                    QString stackedBase = helper::stackedBaseName(file);
                    stacked.insert(stackedBase, QStringList() << file);
                    for (const QString& f : files) {
                        if (helper::stackedBaseName(f) == stackedBase) {
                            stacked[stackedBase].append(f);
                            files.removeOne(f);
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
                    Movie* movie = new Movie(stackedFiles, this);
                    movie->setInSeparateFolder(con.inSeparateFolder);
                    movie->setFileLastModified(m_lastModifications.value(it.value().at(0)));
                    movie->controller()->loadData(Manager::instance()->mediaCenterInterface());
                    movie->setLabel(Manager::instance()->database()->getLabel(movie->files()));
                    Manager::instance()->database()->add(movie, con.path);
                    movies.append(movie);
                    // emit currentDir(movie->name());
                }
            }
            emit progress(++movieCounter, movieSum, m_progressMessageId);
            if (movieCounter % 20 == 0) {
                emit currentDir("");
            }
        }
        Manager::instance()->database()->commit();
    }
    return movies;
}

void MovieFileSearcher::abort()
{
    m_aborted = true;
}
