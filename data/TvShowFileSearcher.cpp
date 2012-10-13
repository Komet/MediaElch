#include "TvShowFileSearcher.h"

#include <QApplication>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "data/TvShowModelItem.h"

/**
 * @brief TvShowFileSearcher::TvShowFileSearcher
 * @param parent
 */
TvShowFileSearcher::TvShowFileSearcher(QObject *parent) :
    QThread(parent)
{
    m_progressMessageId = Constants::TvShowSearcherProgressMessageId;
}

/**
 * @brief Sets the directories
 * @param directories List of directories
 */
void TvShowFileSearcher::setMovieDirectories(QList<SettingsDir> directories)
{
    qDebug() << "Entered";
    m_directories.clear();
    for (int i=0, n=directories.count() ; i<n ; ++i) {
        QFileInfo fi(directories.at(i).path);
        if (fi.isDir()) {
            qDebug() << "Adding tv show directory" << directories.at(i).path << "with mediacenter dir" << directories.at(i).mediaCenterPath;
            m_directories.append(directories.at(i).path);
        }
    }
}

/**
 * @brief Starts the scan process
 */
void TvShowFileSearcher::run()
{
    qDebug() << "Entered";
    emit searchStarted(tr("Searching for TV Shows..."), m_progressMessageId);

    Manager::instance()->tvShowModel()->clear();
    QMap<QString, QList<QStringList> > contents;
    foreach (const QString &path, m_directories) {
        getTvShows(path, contents);
    }
    int i=0;
    int n=0;
    QMapIterator<QString, QList<QStringList> > it(contents);
    while (it.hasNext()) {
        it.next();
        n += it.value().size();
    }
    it.toFront();
    while (it.hasNext()) {
        it.next();
        TvShow *show = new TvShow(it.key());
        show->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
        TvShowModelItem *showItem = Manager::instance()->tvShowModel()->appendChild(show);
        QMap<int, TvShowModelItem*> seasonItems;
        foreach (const QStringList &files, it.value()) {
            TvShowEpisode *episode = new TvShowEpisode(files, show);
            episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
            show->addEpisode(episode);
            if (!seasonItems.contains(episode->season()))
                seasonItems.insert(episode->season(), showItem->appendChild(episode->seasonString(), show));
            seasonItems.value(episode->season())->appendChild(episode);
            emit progress(++i, n, m_progressMessageId);
        }
        show->moveToMainThread();
    }

    qDebug() << "Searching for tv shows done";
    emit tvShowsLoaded(m_progressMessageId);
}

/**
 * @brief Scans a dir for tv show files
 * @param path Directory to scan
 * @param contents
 */
void TvShowFileSearcher::getTvShows(QString path, QMap<QString, QList<QStringList> > &contents)
{
    QDir dir(path);
    QStringList tvShows = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &cDir, tvShows) {
        QList<QStringList> tvShowContents;
        scanTvShowDir(path + QDir::separator() + cDir, tvShowContents);
        contents.insert(QDir::toNativeSeparators(dir.path() + QDir::separator() + cDir), tvShowContents);
    }
}

/**
 * @brief Scans the given path for tv show files.
 * Results are in a list which contains a QStringList for every episode.
 * @param path Path to scan
 * @param contents List of contents
 */
void TvShowFileSearcher::scanTvShowDir(QString path, QList<QStringList> &contents)
{
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
        scanTvShowDir(path + "/" + cDir, contents);
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

    QRegExp rx("((part|cd)[\\s_]*)(\\d+)", Qt::CaseInsensitive);
    for (int i=0, n=files.size() ; i<n ; i++) {
        QStringList tvShowFiles;
        QString file = files.at(i);
        if (file.isEmpty())
            continue;

        tvShowFiles << QDir::toNativeSeparators(path + QDir::separator() + file);

        int pos = rx.indexIn(file);
        if (pos != -1) {
            QString left = file.left(pos) + rx.cap(1);
            QString right = file.mid(pos+rx.cap(1).size()+rx.cap(2).size());
            for (int x=0 ; x<n ; x++) {
                QString subFile = files.at(x);
                if (subFile != file) {
                    if (subFile.startsWith(left) && subFile.endsWith(right)) {
                        tvShowFiles << QDir::toNativeSeparators(path + QDir::separator() + subFile);
                        files[x] = ""; // set an empty file name, this way we can skip this file in the main loop
                    }
                }
            }
        }
        if (tvShowFiles.count() > 0 )
            contents.append(tvShowFiles);
    }
}

/**
 * @brief Get a list of files in a directory
 *        Retrieves the contents from the cache if the last
 *        modification matches the on in the database
 * @param path
 * @return
 */
QStringList TvShowFileSearcher::getCachedFiles(QString path)
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
    query.prepare("SELECT idPath, lastModified FROM tvShowDirs WHERE path=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next()) {
        idPath = query.value(query.record().indexOf("idPath")).toInt();
        if (fi.lastModified() != query.value(query.record().indexOf("lastModified")).toDateTime()) {
            query.prepare("DELETE FROM tvShowDirs WHERE idPath=:idPath");
            query.bindValue(":idPath", idPath);
            query.exec();
            query.prepare("DELETE FROM tvShowFiles WHERE idPath=:idPath");
            query.bindValue(":idPath", idPath);
            query.exec();
            idPath = -1;
        }
    }

    if (idPath != -1) {
        query.prepare("SELECT filename FROM tvShowFiles WHERE idPath=:path");
        query.bindValue(":path", idPath);
        query.exec();
        while (query.next()) {
            files.append(query.value(query.record().indexOf("filename")).toString());
        }
    } else {
        query.prepare("INSERT INTO tvShowDirs(path, lastModified, parent) VALUES(:path, :lastModified, 0)");
        query.bindValue(":path", path);
        query.bindValue(":lastModified", fi.lastModified());
        query.exec();
        idPath = query.lastInsertId().toInt();
        files = QDir(path).entryList(filters, QDir::Files | QDir::System);
        foreach (const QString &file, files) {
            query.prepare("INSERT INTO tvShowFiles(idPath, filename) VALUES(:idPath, :filename)");
            query.bindValue(":idPath", idPath);
            query.bindValue(":filename", file);
            query.exec();
        }
    }

    return files;
}
