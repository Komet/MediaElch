#include "MovieFileSearcher.h"

#include <QApplication>
#include <QDebug>
#include <QDirIterator>
#include <QSqlQuery>
#include <QSqlRecord>
#include "globals/Helper.h"
#include "globals/Manager.h"

/**
 * @brief MovieFileSearcher::MovieFileSearcher
 * @param parent
 */
MovieFileSearcher::MovieFileSearcher(QObject *parent) :
    QObject(parent)
{
    m_progressMessageId = Constants::MovieFileSearcherProgressMessageId;
}

/**
 * @brief MovieFileSearcher::~MovieFileSearcher
 */
MovieFileSearcher::~MovieFileSearcher()
{
}

void MovieFileSearcher::reload(bool force)
{
    m_aborted = false;
    emit searchStarted(tr("Searching for Movies..."), m_progressMessageId);

    if (force)
        Manager::instance()->database()->clearMovies();

    Manager::instance()->movieModel()->clear();
    m_lastModifications.clear();

    QStringList filters;
    filters << "*.mkv" << "*.avi" << "*.mpg" << "*.mpeg" << "*.mp4" << "*.m2ts" << "*.disc" << "*.m4v" << "*.strm"
            << "*.dat" << "*.flv" << "*.vob" << "*.ts" << "*.iso" << "*.ogg" << "*.ogm" << "*.rmvb" << "*.img" << "*.wmv"
            << "*.mov" << "*.divx" << "VIDEO_TS.IFO" << "index.bdmv" << "*.wtv";

    QList<MovieContents> c;
    QList<Movie*> dbMovies;
    QStringList bluRays;
    int movieSum = 0;
    int movieCounter = 0;

    foreach (SettingsDir dir, m_directories) {
        if (m_aborted)
            return;

        QList<Movie*> moviesFromDb;
        if (!dir.autoReload && !force)
            moviesFromDb = Manager::instance()->database()->movies(dir.path);

        if (dir.autoReload || force || moviesFromDb.count() == 0) {
            Manager::instance()->database()->clearMovies(dir.path);
            QMap<QString, QStringList> contents;
            QDirIterator it(dir.path, filters, QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                if (m_aborted)
                    return;
                it.next();
                if (it.fileName().contains("-trailer", Qt::CaseInsensitive) || it.fileName().contains("-sample", Qt::CaseInsensitive))
                    continue;

                // Skip extras folder
                QString dirName = it.fileInfo().dir().dirName();
                if (QString::compare("extras", dirName, Qt::CaseInsensitive) == 0)
                    continue;
                // Skip BluRay backup folder
                if (QString::compare("backup", dirName, Qt::CaseInsensitive) == 0 && QString::compare("index.bdmv", it.fileName(), Qt::CaseInsensitive) == 0)
                    continue;

                if (QString::compare("index.bdmv", it.fileName(), Qt::CaseInsensitive) == 0) {
                    QDir dir(it.fileInfo().dir());
                    dir.cdUp();
                    bluRays << dir.path();
                }

                QString path = it.fileInfo().path();
                if (!contents.contains(path))
                    contents.insert(path, QStringList());
                contents[path].append(it.filePath());
                m_lastModifications.insert(it.filePath(), it.fileInfo().lastModified());
                emit currentDir("/" + it.fileInfo().dir().dirName());
            }
            movieSum += contents.count();
            MovieContents con;
            con.path = dir.path;
            con.inSeparateFolder = dir.separateFolders;
            con.contents = contents;
            c.append(con);
        } else {
            dbMovies.append(moviesFromDb);
            movieSum += moviesFromDb.count();
        }
    }

    emit searchStarted(tr("Loading Movies..."), m_progressMessageId);

    QList<Movie*> movies;
    foreach (const MovieContents &con, c) {
        Manager::instance()->database()->transaction();
        QMapIterator<QString, QStringList> itContents(con.contents);
        while (itContents.hasNext()) {
            if (m_aborted) {
                Manager::instance()->database()->commit();
                return;
            }
            itContents.next();
            QStringList files = itContents.value();

            // BluRay handling
            foreach (const QString &path, bluRays) {
                if (!files.isEmpty() && files.first().startsWith(path)) {
                    QStringList f;
                    foreach (const QString &file, files) {
                        if (file.endsWith("index.bdmv", Qt::CaseInsensitive))
                            f.append(file);
                    }
                    files = f;
                }
            }

            // DVD handling
            if (files.count() > 1) {
                foreach (const QString &file, files) {
                    if (file.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
                        files = QStringList() << file;
                        break;
                    }
                }
            }

            if (files.isEmpty())
                continue;

            if (files.count() == 1 || con.inSeparateFolder) {
                // single file or in separate folder
                Movie *movie = new Movie(files, this);
                movie->setInSeparateFolder(con.inSeparateFolder);
                movie->setFileLastModified(m_lastModifications.value(files.at(0)));
                movie->controller()->loadData(Manager::instance()->mediaCenterInterface());
                Manager::instance()->database()->add(movie, con.path);
                movies.append(movie);
                emit currentDir(movie->name());
            } else {
                QMap<QString, QStringList> stacked;
                while (!files.isEmpty()) {
                    QString file = files.takeLast();
                    QString stackedBase = Helper::stackedBaseName(file);
                    stacked.insert(stackedBase, QStringList() << file);
                    foreach (const QString &f, files) {
                        if (Helper::stackedBaseName(f) == stackedBase) {
                            stacked[stackedBase].append(f);
                            files.removeOne(f);
                        }
                    }
                }
                QMapIterator<QString, QStringList> it(stacked);
                while (it.hasNext()) {
                    it.next();
                    if (it.value().isEmpty())
                        continue;
                    Movie *movie = new Movie(it.value(), this);
                    movie->setInSeparateFolder(con.inSeparateFolder);
                    movie->setFileLastModified(m_lastModifications.value(it.value().at(0)));
                    movie->controller()->loadData(Manager::instance()->mediaCenterInterface());
                    Manager::instance()->database()->add(movie, con.path);
                    movies.append(movie);
                    emit currentDir(movie->name());
                }
            }
            emit progress(++movieCounter, movieSum, m_progressMessageId);
        }
        Manager::instance()->database()->commit();
    }

    emit currentDir("");

    foreach (Movie *movie, dbMovies) {
        if (m_aborted)
            return;
        movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), false, false);
        movies.append(movie);
        emit currentDir(movie->name());
        emit progress(++movieCounter, movieSum, m_progressMessageId);
    }

    foreach (Movie *movie, movies)
        Manager::instance()->movieModel()->addMovie(movie);

    if (!m_aborted)
        emit moviesLoaded(m_progressMessageId);
}

/**
 * @brief Sets the directories to scan for movies. Not existing directories are skipped.
 * @param directories List of directories
 */
void MovieFileSearcher::setMovieDirectories(QList<SettingsDir> directories)
{
    qDebug() << "Entered";
    m_directories.clear();
    for (int i=0, n=directories.count() ; i<n ; ++i) {
        QFileInfo fi(directories.at(i).path);
        if (fi.isDir()) {
            qDebug() << "Adding movie directory" << directories.at(i).path;
            m_directories.append(directories.at(i));
        }
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
void MovieFileSearcher::scanDir(QString startPath, QString path, QList<QStringList> &contents, bool separateFolders, bool firstScan)
{
    emit currentDir(path.mid(startPath.length()));

    QDir dir(path);
    foreach (const QString &cDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (m_aborted)
            return;

        // Skip "Extras" folder
        if (QString::compare(cDir, "Extras", Qt::CaseInsensitive) == 0 ||
            QString::compare(cDir, ".actors", Qt::CaseInsensitive) == 0 ||
            QString::compare(cDir, ".AppleDouble", Qt::CaseInsensitive) == 0 ||
            QString::compare(cDir, "extrafanarts", Qt::CaseInsensitive) == 0)
            continue;

        // Handle DVD
        if (Helper::isDvd(path + QDir::separator() + cDir)) {
            contents.append(QStringList() << QDir::toNativeSeparators(path + "/" + cDir + "/VIDEO_TS/VIDEO_TS.IFO"));
            continue;
        }

        // Handle BluRay
        if (Helper::isBluRay(path + QDir::separator() + cDir)) {
            contents.append(QStringList() << QDir::toNativeSeparators(path + "/" + cDir + "/BDMV/index.bdmv"));
            continue;
        }

        // Don't scan subfolders when separate folders is checked
        if (!separateFolders || firstScan)
            scanDir(startPath, path + "/" + cDir, contents, separateFolders);
    }

    QStringList files;
    QStringList entries = getFiles(path);
    foreach (const QString &file, entries) {
        if (m_aborted)
            return;

        // Skip Trailers and Sample files
        if (file.contains("-trailer", Qt::CaseInsensitive) || file.contains("-sample", Qt::CaseInsensitive))
            continue;
        files.append(file);
    }
    files.sort();

    if (separateFolders) {
        QStringList movieFiles;
        foreach (const QString &file, files)
            movieFiles.append(QDir::toNativeSeparators(path + "/" + file));
        if (movieFiles.count() > 0)
            contents.append(movieFiles);
        return;
    }

    /* detect movies with multiple files*/
    QRegExp rx("([\\-_\\s\\.\\(\\)]+((a|b|c|d|e|f)|((part|cd|xvid)" \
               "[\\-_\\s\\.\\(\\)]*\\d+))[\\-_\\s\\.\\(\\)]+)",
               Qt::CaseInsensitive);
    for (int i=0, n=files.size() ; i<n ; i++) {
        if (m_aborted)
            return;

        QStringList movieFiles;
        QString file = files.at(i);
        if (file.isEmpty())
            continue;

        movieFiles << QDir::toNativeSeparators(path + QDir::separator() + file);

        int pos = rx.lastIndexIn(file);
        if (pos != -1) {
            QString left = file.left(pos);
            QString right = file.mid(pos + rx.cap(0).size());
            for (int x=0 ; x<n ; x++) {
                QString subFile = files.at(x);
                if (subFile != file) {
                    if (subFile.startsWith(left) && subFile.endsWith(right)) {
                        movieFiles << QDir::toNativeSeparators(path + QDir::separator() + subFile);
                        files[x] = ""; // set an empty file name, this way we can skip this file in the main loop
                    }
                }
            }
        }
        if (movieFiles.count() > 0 )
            contents.append(movieFiles);
    }
}

/**
 * @brief Get a list of files in a directory
 * @param path
 * @return
 */
QStringList MovieFileSearcher::getFiles(QString path)
{
    QStringList files;
    QStringList filters;
    filters << "*.mkv" << "*.avi" << "*.mpg" << "*.mpeg" << "*.mp4" << "*.m2ts" << "*.disc" << "*.m4v" << "*.strm"
            << "*.dat" << "*.flv" << "*.vob" << "*.ts" << "*.iso" << "*.ogg" << "*.ogm" << "*.rmvb" << "*.img" << "*.wmv"
            << "*.mov" << "*.divx" << "*.wtv";

    foreach (const QString &file, QDir(path).entryList(filters, QDir::Files | QDir::System)) {
        m_lastModifications.insert(QDir::toNativeSeparators(path + "/" + file),
                                   QFileInfo(path + QDir::separator() + file).lastModified());
        files.append(file);
    }
    return files;
}

void MovieFileSearcher::abort()
{
    m_aborted = true;
}
