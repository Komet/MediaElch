#include "MovieFileSearcher.h"

#include <QApplication>
#include <QDebug>
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

/**
 * @brief Starts the scanning process
 */
void MovieFileSearcher::reload(bool force)
{
    if (force)
        Manager::instance()->database()->clearMovies();

    Manager::instance()->movieModel()->clear();
    m_lastModifications.clear();

    emit searchStarted(tr("Searching for Movies..."), m_progressMessageId);
    QList<Movie*> dbMovies;
    QList<QStringList> contents;
    foreach (SettingsDir dir, m_directories) {
        QList<Movie*> moviesFromDb = Manager::instance()->database()->movies(dir.path);
        if (dir.autoReload || force || moviesFromDb.count() == 0) {
            Manager::instance()->database()->clearMovies(dir.path);
            scanDir(dir.path, dir.path, contents, dir.separateFolders, true);
        } else {
            dbMovies.append(moviesFromDb);
        }
    }
    emit currentDir("");

    emit searchStarted(tr("Loading Movies..."), m_progressMessageId);
    int movieCounter=0;
    int movieSum=contents.size()+dbMovies.size();

    // Setup movies
    foreach (const QStringList &files, contents) {
        bool inSeparateFolder = false;
        QString path;
        // get directory
        if (!files.isEmpty()) {
            int index = -1;
            for (int i=0, n=m_directories.count() ; i<n ; ++i) {
                if (files.at(0).startsWith(m_directories[i].path)) {
                    if (index == -1)
                        index = i;
                    else if (m_directories[index].path.length() < m_directories[i].path.length())
                        index = i;
                }
            }
            if (index != -1) {
                inSeparateFolder = m_directories[index].separateFolders;
                path = m_directories[index].path;
            }
        }
        Movie *movie = new Movie(files, this);
        movie->setInSeparateFolder(inSeparateFolder);
        movie->loadData(Manager::instance()->mediaCenterInterface());
        emit currentDir(movie->name());
        if (!files.isEmpty())
            movie->setFileLastModified(m_lastModifications.value(files.at(0)));

        Manager::instance()->database()->add(movie, path);
        Manager::instance()->movieModel()->addMovie(movie);
        emit progress(++movieCounter, movieSum, m_progressMessageId);
        qApp->processEvents();
    }

    // Setup movies loaded from database
    foreach (Movie *movie, dbMovies) {
        movie->loadData(Manager::instance()->mediaCenterInterface(), false, false);
        emit currentDir(movie->name());
        Manager::instance()->movieModel()->addMovie(movie);
        emit progress(++movieCounter, movieSum, m_progressMessageId);
        qApp->processEvents();
    }

    qDebug() << "Searching for movies done";
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
            qDebug() << "Adding movie directory" << directories.at(i).path << "with mediacenter dir" << directories.at(i).mediaCenterPath;
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
 */
void MovieFileSearcher::scanDir(QString startPath, QString path, QList<QStringList> &contents, bool separateFolders, bool firstScan)
{
    emit currentDir(path.mid(startPath.length()));

    QDir dir(path);
    foreach (const QString &cDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        // Skip "Extras" folder
        if (QString::compare(cDir, "Extras", Qt::CaseInsensitive) == 0 ||
            QString::compare(cDir, ".actors", Qt::CaseInsensitive) == 0)
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
            << "*.dat" << "*.flv" << "*.vob" << "*.ts" << "*.iso" << "*.ogg" << "*.ogm" << "*.rmvb";

    foreach (const QString &file, QDir(path).entryList(filters, QDir::Files | QDir::System)) {
        m_lastModifications.insert(QDir::toNativeSeparators(path + "/" + file),
                                   QFileInfo(path + QDir::separator() + file).lastModified());
        files.append(file);
    }
    return files;
}
