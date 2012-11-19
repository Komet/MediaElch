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
void ConcertFileSearcher::reload(bool force)
{
    if (force)
        Manager::instance()->database()->clearConcerts();

    Manager::instance()->concertModel()->clear();
    emit searchStarted(tr("Searching for Concerts..."), m_progressMessageId);

    QList<Concert*> dbConcerts;
    QList<QStringList> contents;
    foreach (SettingsDir dir, m_directories) {
        QList<Concert*> concertsFromDb = Manager::instance()->database()->concerts(dir.path);
        if (dir.autoReload || force || concertsFromDb.count() == 0) {
            Manager::instance()->database()->clearConcerts(dir.path);
            scanDir(dir.path, dir.path, contents, dir.separateFolders, true);
        } else {
            dbConcerts.append(concertsFromDb);
        }
    }
    emit currentDir("");

    emit searchStarted(tr("Loading Concerts..."), m_progressMessageId);
    int concertCounter=0;
    int concertSum=contents.size()+dbConcerts.size();

    // Setup concerts
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
        Concert *concert = new Concert(files, this);
        concert->setInSeparateFolder(inSeparateFolder);
        concert->loadData(Manager::instance()->mediaCenterInterface());
        emit currentDir(concert->name());
        Manager::instance()->database()->add(concert, path);
        Manager::instance()->concertModel()->addConcert(concert);
        emit progress(++concertCounter, concertSum, m_progressMessageId);
        qApp->processEvents();
    }

    // Setup concerts loaded from database
    foreach (Concert *concert, dbConcerts) {
        concert->loadData(Manager::instance()->mediaCenterInterface(), false, false);
        emit currentDir(concert->name());
        Manager::instance()->concertModel()->addConcert(concert);
        emit progress(++concertCounter, concertSum, m_progressMessageId);
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
 * @param path
 * @return
 */
QStringList ConcertFileSearcher::getFiles(QString path)
{
    QStringList filters;
    filters << "*.mkv" << "*.avi" << "*.mpg" << "*.mpeg" << "*.mp4" << "*.m2ts" << "*.disc" << "*.m4v" << "*.strm"
            << "*.dat" << "*.flv" << "*.vob" << "*.ts" << "*.rmvb";

    return QDir(path).entryList(filters, QDir::Files | QDir::System);
}
