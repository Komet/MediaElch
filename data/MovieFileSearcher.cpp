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
void MovieFileSearcher::run()
{
    qDebug() << "Entered";
    emit searchStarted(tr("Searching for Movies..."), m_progressMessageId);

    Manager::instance()->movieModel()->clear();
    m_lastModifications.clear();
    QList<QStringList> contents;
    foreach (SettingsDir dir, m_directories)
        scanDir(dir.path, dir.path, contents, dir.separateFolders, true);

    emit currentDir("");

    int i=0;
    int n=contents.size();
    foreach (const QStringList &files, contents) {
        bool inSeparateFolder = false;
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
            if (index != -1)
                inSeparateFolder = m_directories[index].separateFolders;
        }
        Movie *movie = new Movie(files, this);
        movie->setInSeparateFolder(inSeparateFolder);
        movie->loadData(Manager::instance()->mediaCenterInterface());
        if (!files.isEmpty())
            movie->setFileLastModified(m_lastModifications.value(files.at(0)));
        Manager::instance()->movieModel()->addMovie(movie);
        emit progress(++i, n, m_progressMessageId);
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
        if (QString::compare(cDir, "Extras", Qt::CaseInsensitive) == 0)
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
    QStringList entries = getCachedFiles(path);
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
 *        Retrieves the contents from the cache if the last
 *        modification matches the on in the database
 * @param path
 * @return
 */
QStringList MovieFileSearcher::getCachedFiles(QString path)
{
    QStringList files;
    QStringList filters;
    filters << "*.mkv" << "*.avi" << "*.mpg" << "*.mpeg" << "*.mp4" << "*.m2ts" << "*.disc" << "*.m4v" << "*.strm"
            << "*.dat" << "*.flv" << "*.vob" << "*.ts" << "*.iso" << "*.ogg" << "*.ogm";

    if (!Settings::instance()->useCache()) {
        foreach (const QString &file, QDir(path).entryList(filters, QDir::Files | QDir::System)) {
            m_lastModifications.insert(QDir::toNativeSeparators(path + "/" + file),
                                       QFileInfo(path + QDir::separator() + file).lastModified());
            files.append(file);
        }
        return files;
    }

    int idPath = -1;
    QFileInfo fi(path);
    QSqlQuery query(Manager::instance()->cacheDb());
    query.prepare("SELECT idPath, lastModified FROM movieDirs WHERE path=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next()) {
        idPath = query.value(query.record().indexOf("idPath")).toInt();
        if (fi.lastModified() != query.value(query.record().indexOf("lastModified")).toDateTime()) {
            query.prepare("DELETE FROM movieDirs WHERE idPath=:idPath");
            query.bindValue(":idPath", idPath);
            query.exec();
            query.prepare("DELETE FROM movieFiles WHERE idPath=:idPath");
            query.bindValue(":idPath", idPath);
            query.exec();
            idPath = -1;
        }
    }

    if (idPath != -1) {
        query.prepare("SELECT filename, lastModified FROM movieFiles WHERE idPath=:path");
        query.bindValue(":path", idPath);
        query.exec();
        while (query.next()) {
            m_lastModifications.insert(QDir::toNativeSeparators(path + "/" + query.value(query.record().indexOf("filename")).toString()),
                                       query.value(query.record().indexOf("lastModified")).toDateTime());
            files.append(query.value(query.record().indexOf("filename")).toString());
        }
    } else {
        query.prepare("INSERT INTO movieDirs(path, lastModified, parent) VALUES(:path, :lastModified, 0)");
        query.bindValue(":path", path);
        query.bindValue(":lastModified", fi.lastModified());
        query.exec();
        idPath = query.lastInsertId().toInt();
        files = QDir(path).entryList(filters, QDir::Files | QDir::System);
        foreach (const QString &file, files) {
            QDateTime lastMod = QFileInfo(path + QDir::separator() + file).lastModified();

            query.prepare("INSERT INTO movieFiles(idPath, filename, lastModified) VALUES(:idPath, :filename, :lastModified)");
            query.bindValue(":idPath", idPath);
            query.bindValue(":filename", file);
            query.bindValue(":lastModified", lastMod);
            query.exec();

            m_lastModifications.insert(QDir::toNativeSeparators(path + "/" + file), lastMod);
            files.append(file);
        }
    }

    return files;
}
