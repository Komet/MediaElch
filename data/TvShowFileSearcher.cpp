#include "TvShowFileSearcher.h"

#include <QApplication>
#include <QFileInfo>
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
                seasonItems.insert(episode->season(), showItem->appendChild(episode->seasonString()));
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
    QStringList filters;
    filters << "*.mkv" << "*.avi" << "*.mpg" << "*.mpeg" << "*.mp4" << "*.m2ts" << "VIDEO_TS.ifo" << "index.bdmv" << "*.disc" << "*.m4v" << "*.strm";

    QDir dir(path);
    foreach (const QString &cDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDir tvShowDir = QDir(path + QDir::separator() + cDir);
        QStringList subDirs;
        subDirs << tvShowDir.path();
        getSubDirs(tvShowDir, subDirs);

        foreach (const QString &subDir, subDirs) {
            if (subDir.endsWith("BDMV/BACKUP", Qt::CaseInsensitive) || subDir.endsWith("BDMV\\Backup", Qt::CaseInsensitive) ||
                subDir.endsWith("BDMV/STREAM", Qt::CaseInsensitive) || subDir.endsWith("BDMV\\STREAM", Qt::CaseInsensitive) ||
                subDir.contains( "Extras", Qt::CaseInsensitive))
                continue;

            QStringList files;
            foreach (const QString &file, QDir(subDir).entryList(filters, QDir::Files | QDir::System)) {
                files.append(file);
            }
            files.sort();

            QRegExp rx("((part|cd)[\\s_]*)(\\d+)", Qt::CaseInsensitive);
            for (int i=0, n=files.size() ; i<n ; ++i) {
                QStringList tvShowFiles;
                QString file = files.at(i);
                if (file.isEmpty())
                    continue;
                tvShowFiles << QDir::toNativeSeparators(subDir) + QDir::separator() + file;
                int pos = rx.indexIn(file);
                if (pos != -1) {
                    QString left = file.left(pos) + rx.cap(1);
                    QString right = file.mid(pos+rx.cap(1).size()+rx.cap(2).size());
                    for (int x=0 ; x<n ; x++) {
                        QString subFile = files.at(x);
                        if (subFile != file) {
                            if (subFile.startsWith(left) && subFile.endsWith(right)) {
                                tvShowFiles << QDir::toNativeSeparators(subDir) + QDir::separator() + subFile;
                                files[x] = ""; // set an empty file name, this way we can skip this file in the main loop
                            }
                        }
                    }
                }

                if (contents.contains(QDir::toNativeSeparators(dir.path() + QDir::separator() + cDir))) {
                    qDebug() << "Appending tv show" << QDir::toNativeSeparators(dir.path() + QDir::separator() + cDir) << tvShowFiles;
                    contents[QDir::toNativeSeparators(dir.path() + QDir::separator() + cDir)].append(tvShowFiles);
                } else {
                    QList<QStringList> l;
                    l << tvShowFiles;
                    qDebug() << "Inserting tv show" << QDir::toNativeSeparators(dir.path() + QDir::separator() + cDir) << l;
                    contents.insert(QDir::toNativeSeparators(dir.path() + QDir::separator() + cDir), l);
                }
            }
        }
    }
}

/**
 * @brief Returns a list of all subdirectories
 * @param dir Dir to scan
 * @param subDirs
 */
void TvShowFileSearcher::getSubDirs(QDir dir, QStringList &subDirs)
{
    foreach (const QString &cDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        subDirs.append(dir.path() + QDir::separator() + cDir);
        getSubDirs(QDir(dir.path() + QDir::separator() + cDir), subDirs);
    }
}
