#include "ConcertFileSearcher.h"

#include <QApplication>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>
#include "globals/Helper.h"
#include "globals/Manager.h"

/**
 * @brief ConcertFileSearcher::ConcertFileSearcher
 * @param parent
 */
ConcertFileSearcher::ConcertFileSearcher(QObject *parent) :
    QObject(parent)
{
    m_progressMessageId = Constants::ConcertFileSearcherProgressMessageId;
}

/**
 * @brief Sets the directories to scan for concerts. Not existing directories are skipped.
 * @param directories List of directories
 */
void ConcertFileSearcher::setConcertDirectories(QList<SettingsDir> directories)
{
    qDebug() << "Entered";
    m_directories.clear();
    for (int i=0, n=directories.count() ; i<n ; ++i) {
        QFileInfo fi(directories.at(i).path);
        if (fi.isDir()) {
            qDebug() << "Adding concert directory" << directories.at(i).path << "with mediacenter dir" << directories.at(i).mediaCenterPath;
            m_directories.append(directories.at(i));
        }
    }
}

/**
 * @brief Starts the scanning process
 */
void ConcertFileSearcher::run()
{
    qDebug() << "Entered";
    emit searchStarted(tr("Searching for Concerts..."), m_progressMessageId);

    Manager::instance()->concertModel()->clear();
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
        Concert *concert = new Concert(files, this);
        concert->setInSeparateFolder(inSeparateFolder);
        concert->loadData(Manager::instance()->mediaCenterInterface());
        Manager::instance()->concertModel()->addConcert(concert);
        emit progress(++i, n, m_progressMessageId);
        qApp->processEvents();
    }

    qDebug() << "Searching for concerts done";

    emit concertsLoaded(m_progressMessageId);
}

/**
 * @brief Scans the given path for concert files.
 * Results are in a list which contains a QStringList for every concert.
 * @param startPath Scanning started at this path
 * @param path Path to scan
 * @param contents List of contents
 * @param separateFolders Are concerts in separate folders
 * @param firstScan When this is true, subfolders are scanned, regardless of separateFolders
 */
void ConcertFileSearcher::scanDir(QString startPath, QString path, QList<QStringList> &contents, bool separateFolders, bool firstScan)
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
        // Skip Trailers
        if (file.contains("-trailer", Qt::CaseInsensitive))
            continue;
        files.append(file);
    }
    files.sort();

    if (separateFolders) {
        QStringList concertFiles;
        foreach (const QString &file, files)
            concertFiles.append(QDir::toNativeSeparators(path + "/" + file));
        if (concertFiles.count() > 0)
            contents.append(concertFiles);
        return;
    }

    QRegExp rx("((part|cd)[\\s_]*)(\\d+)", Qt::CaseInsensitive);
    for (int i=0, n=files.size() ; i<n ; i++) {
        QStringList concertFiles;
        QString file = files.at(i);
        if (file.isEmpty())
            continue;

        concertFiles << QDir::toNativeSeparators(path + QDir::separator() + file);

        int pos = rx.indexIn(file);
        if (pos != -1) {
            QString left = file.left(pos) + rx.cap(1);
            QString right = file.mid(pos+rx.cap(1).size()+rx.cap(2).size());
            for (int x=0 ; x<n ; x++) {
                QString subFile = files.at(x);
                if (subFile != file) {
                    if (subFile.startsWith(left) && subFile.endsWith(right)) {
                        concertFiles << QDir::toNativeSeparators(path + QDir::separator() + subFile);
                        files[x] = ""; // set an empty file name, this way we can skip this file in the main loop
                    }
                }
            }
        }
        if (concertFiles.count() > 0 )
            contents.append(concertFiles);
    }
}

/**
 * @brief Get a list of files in a directory
 *        Retrieves the contents from the cache if the last
 *        modification matches the on in the database
 * @param path
 * @return
 */
QStringList ConcertFileSearcher::getCachedFiles(QString path)
{
    QStringList filters;
    filters << "*.mkv" << "*.avi" << "*.mpg" << "*.mpeg" << "*.mp4" << "*.m2ts" << "*.disc" << "*.m4v" << "*.strm"
            << "*.dat" << "*.flv" << "*.vob" << "*.ts";

    if (!Settings::instance()->useCache())
        return QDir(path).entryList(filters, QDir::Files | QDir::System);

    int idPath = -1;
    QStringList files;
    QFileInfo fi(path);
    QSqlQuery query(Manager::instance()->cacheDb());
    query.prepare("SELECT idPath, lastModified FROM concertDirs WHERE path=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next()) {
        idPath = query.value(query.record().indexOf("idPath")).toInt();
        if (fi.lastModified() != query.value(query.record().indexOf("lastModified")).toDateTime()) {
            query.prepare("DELETE FROM concertDirs WHERE idPath=:idPath");
            query.bindValue(":idPath", idPath);
            query.exec();
            query.prepare("DELETE FROM concertFiles WHERE idPath=:idPath");
            query.bindValue(":idPath", idPath);
            query.exec();
            idPath = -1;
        }
    }

    if (idPath != -1) {
        query.prepare("SELECT filename FROM concertFiles WHERE idPath=:path");
        query.bindValue(":path", idPath);
        query.exec();
        while (query.next()) {
            files.append(query.value(query.record().indexOf("filename")).toString());
        }
    } else {
        query.prepare("INSERT INTO concertDirs(path, lastModified, parent) VALUES(:path, :lastModified, 0)");
        query.bindValue(":path", path);
        query.bindValue(":lastModified", fi.lastModified());
        query.exec();
        idPath = query.lastInsertId().toInt();
        files = QDir(path).entryList(filters, QDir::Files | QDir::System);
        foreach (const QString &file, files) {
            query.prepare("INSERT INTO concertFiles(idPath, filename) VALUES(:idPath, :filename)");
            query.bindValue(":idPath", idPath);
            query.bindValue(":filename", file);
            query.exec();
        }
    }

    return files;
}
