#include "TvShowFileSearcher.h"

#include <QApplication>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QtConcurrent/QtConcurrentMap>
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
    QObject(parent)
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
            qDebug() << "Adding tv show directory" << directories.at(i).path;
            m_directories.append(directories.at(i));
        }
    }
}

/**
 * @brief Starts the scan process
 */
void TvShowFileSearcher::reload(bool force)
{
    m_aborted = false;

    if (force)
        Manager::instance()->database()->clearTvShows();

    emit searchStarted(tr("Searching for TV Shows..."), m_progressMessageId);
    QList<TvShow*> dbShows;
    Manager::instance()->tvShowModel()->clear();
    Manager::instance()->tvShowFilesWidget()->renewModel();
    QMap<QString, QList<QStringList> > contents;
    foreach (SettingsDir dir, m_directories) {
        if (m_aborted)
            return;

        QList<TvShow*> showsFromDatabase = Manager::instance()->database()->shows(dir.path);
        if (dir.autoReload || force || showsFromDatabase.count() == 0) {
            Manager::instance()->database()->clearTvShows(dir.path);
            getTvShows(dir.path, contents);
        } else {
            dbShows.append(showsFromDatabase);
        }
    }
    emit currentDir("");

    emit searchStarted(tr("Loading TV Shows..."), m_progressMessageId);
    int episodeCounter=0;
    int episodeSum=Manager::instance()->database()->episodeCount();
    QMapIterator<QString, QList<QStringList> > it(contents);
    while (it.hasNext()) {
        it.next();
        episodeSum += it.value().size();
    }
    it.toFront();

    // Setup shows
    while (it.hasNext()) {
        if (m_aborted)
            return;

        it.next();

        // get path
        QString path;
        int index = -1;
        for (int i=0, n=m_directories.count() ; i<n ; ++i) {
            if (it.key().startsWith(m_directories[i].path)) {
                if (index == -1)
                    index = i;
                else if (m_directories[index].path.length() < m_directories[i].path.length())
                    index = i;
            }
        }
        if (index != -1) {
            path = m_directories[index].path;
        }

        TvShow *show = new TvShow(it.key(), this);
        show->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
        emit currentDir(show->name());
        Manager::instance()->database()->add(show, path);
        TvShowModelItem *showItem = Manager::instance()->tvShowModel()->appendChild(show);

        Manager::instance()->database()->transaction();
        QMap<int, TvShowModelItem*> seasonItems;
        QList<TvShowEpisode*> episodes;

        // Setup episodes list
        foreach (const QStringList &files, it.value()) {
            int seasonNumber = getSeasonNumber(files);
            QList<int> episodeNumbers = getEpisodeNumbers(files);
            foreach (const int &episodeNumber, episodeNumbers) {
                TvShowEpisode *episode = new TvShowEpisode(files, show);
                episode->setSeason(seasonNumber);
                episode->setEpisode(episodeNumber);
                episodes.append(episode);
            }
        }

        // Load episodes data
        QtConcurrent::blockingMapped(episodes, TvShowFileSearcher::reloadEpisodeData);

        // Add episodes to model
        foreach (TvShowEpisode *episode, episodes) {
            Manager::instance()->database()->add(episode, path, show->databaseId());
            show->addEpisode(episode);
            if (!seasonItems.contains(episode->season()))
                seasonItems.insert(episode->season(), showItem->appendChild(episode->season(), episode->seasonString(), show));
            seasonItems.value(episode->season())->appendChild(episode);
            emit progress(++episodeCounter, episodeSum, m_progressMessageId);
        }

        Manager::instance()->database()->commit();
    }

    emit currentDir("");

    // Setup shows loaded from database
    foreach (TvShow *show, dbShows) {
        if (m_aborted)
            return;

        show->loadData(Manager::instance()->mediaCenterInterfaceTvShow(), false);
        TvShowModelItem *showItem = Manager::instance()->tvShowModel()->appendChild(show);

        QMap<int, TvShowModelItem*> seasonItems;
        QList<TvShowEpisode*> episodes = Manager::instance()->database()->episodes(show->databaseId());
        QtConcurrent::blockingMapped(episodes, TvShowFileSearcher::loadEpisodeData);
        foreach (TvShowEpisode *episode, episodes) {
            episode->setShow(show);
            show->addEpisode(episode);
            if (!seasonItems.contains(episode->season()))
                seasonItems.insert(episode->season(), showItem->appendChild(episode->season(), episode->seasonString(), show));
            seasonItems.value(episode->season())->appendChild(episode);
            emit progress(++episodeCounter, episodeSum, m_progressMessageId);
            if (episodeCounter%1000 == 0)
                emit currentDir("");
        }
    }

    foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows()) {
        if (show->showMissingEpisodes())
            show->fillMissingEpisodes();
    }

    qDebug() << "Searching for tv shows done";
    if (!m_aborted)
        emit tvShowsLoaded(m_progressMessageId);
}

TvShowEpisode *TvShowFileSearcher::loadEpisodeData(TvShowEpisode *episode)
{
    episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow(), false);
    return episode;
}

void TvShowFileSearcher::reloadEpisodes(QString showDir)
{
    Manager::instance()->database()->clearTvShow(showDir);
    emit searchStarted(tr("Searching for Episodes..."), m_progressMessageId);

    // remove old show object
    foreach (TvShow *s, Manager::instance()->tvShowModel()->tvShows()) {
        if (m_aborted)
            return;

        if (s->dir() == showDir) {
            Manager::instance()->tvShowModel()->removeShow(s);
            break;
        }
    }

    // get path
    QString path;
    int index = -1;
    for (int i=0, n=m_directories.count() ; i<n ; ++i) {
        if (m_aborted)
            return;

        if (showDir.startsWith(m_directories[i].path)) {
            if (index == -1)
                index = i;
            else if (m_directories[index].path.length() < m_directories[i].path.length())
                index = i;
        }
    }
    if (index != -1)
        path = m_directories[index].path;

    // search for contents
    QList<QStringList> contents;
    scanTvShowDir(path, showDir, contents);
    TvShow *show = new TvShow(showDir, this);
    show->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
    Manager::instance()->database()->add(show, path);
    TvShowModelItem *showItem = Manager::instance()->tvShowModel()->appendChild(show);

    emit searchStarted(tr("Loading Episodes..."), m_progressMessageId);
    emit currentDir(show->name());

    int episodeCounter = 0;
    int episodeSum = contents.count();
    QMap<int, TvShowModelItem*> seasonItems;
    QList<TvShowEpisode*> episodes;
    foreach (const QStringList &files, contents) {
        if (m_aborted)
            return;
        int seasonNumber = getSeasonNumber(files);
        QList<int> episodeNumbers = getEpisodeNumbers(files);
        foreach (const int &episodeNumber, episodeNumbers) {
            TvShowEpisode *episode = new TvShowEpisode(files, show);
            episode->setSeason(seasonNumber);
            episode->setEpisode(episodeNumber);
            episodes.append(episode);
        }
    }

    QtConcurrent::blockingMapped(episodes, TvShowFileSearcher::reloadEpisodeData);

    foreach (TvShowEpisode *episode, episodes) {
        Manager::instance()->database()->add(episode, path, show->databaseId());
        show->addEpisode(episode);
        if (!seasonItems.contains(episode->season()))
            seasonItems.insert(episode->season(), showItem->appendChild(episode->season(), episode->seasonString(), show));
        seasonItems.value(episode->season())->appendChild(episode);
        emit progress(++episodeCounter, episodeSum, m_progressMessageId);
        qApp->processEvents();
    }

    emit tvShowsLoaded(m_progressMessageId);
}

TvShowEpisode *TvShowFileSearcher::reloadEpisodeData(TvShowEpisode *episode)
{
    episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
    return episode;
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
        if (m_aborted)
            return;

        QList<QStringList> tvShowContents;
        scanTvShowDir(path, path + QDir::separator() + cDir, tvShowContents);
        contents.insert(QDir::toNativeSeparators(dir.path() + QDir::separator() + cDir), tvShowContents);
    }
}

/**
 * @brief Scans the given path for tv show files.
 * Results are in a list which contains a QStringList for every episode.
 * @param startPath Scanning started at this path
 * @param path Path to scan
 * @param contents List of contents
 */
void TvShowFileSearcher::scanTvShowDir(QString startPath, QString path, QList<QStringList> &contents)
{
    emit currentDir(path.mid(startPath.length()));

    QDir dir(path);
    foreach (const QString &cDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (m_aborted)
            return;

        // Skip "Extras" folder
        if (QString::compare(cDir, "Extras", Qt::CaseInsensitive) == 0 ||
            QString::compare(cDir, ".actors", Qt::CaseInsensitive) == 0 ||
            QString::compare(cDir, "extrafanarts", Qt::CaseInsensitive) == 0)
            continue;

        // Handle DVD
        if (Helper::instance()->isDvd(path + QDir::separator() + cDir)) {
            contents.append(QStringList() << QDir::toNativeSeparators(path + "/" + cDir + "/VIDEO_TS/VIDEO_TS.IFO"));
            continue;
        }
        if (Helper::instance()->isDvd(path + QDir::separator() + cDir, true)) {
            contents.append(QStringList() << QDir::toNativeSeparators(path + "/" + cDir + "/VIDEO_TS.IFO"));
            continue;
        }

        // Handle BluRay
        if (Helper::instance()->isBluRay(path + QDir::separator() + cDir)) {
            contents.append(QStringList() << QDir::toNativeSeparators(path + "/" + cDir + "/BDMV/index.bdmv"));
            continue;
        }
        scanTvShowDir(startPath, path + "/" + cDir, contents);
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

    QRegExp rx("((part|cd)[\\s_]*)(\\d+)", Qt::CaseInsensitive);
    for (int i=0, n=files.size() ; i<n ; i++) {
        if (m_aborted)
            return;

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
 * @param path
 * @return
 */
QStringList TvShowFileSearcher::getFiles(QString path)
{
    if (Settings::instance()->advanced()->tvShowFilters().isEmpty())
        return QStringList();

    return QDir(path).entryList(Settings::instance()->advanced()->tvShowFilters(), QDir::Files | QDir::System);
}

void TvShowFileSearcher::abort()
{
    m_aborted = true;
}

int TvShowFileSearcher::getSeasonNumber(QStringList files)
{
    if (files.isEmpty())
        return -2;

    QStringList filenameParts = files.at(0).split(QDir::separator());
    QString filename = filenameParts.last();
    if (filename.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 1 && Helper::instance()->isDvd(files.at(0)))
            filename = filenameParts.at(filenameParts.count()-3);
        else if (filenameParts.count() > 2 && Helper::instance()->isDvd(files.at(0), true))
            filename = filenameParts.at(filenameParts.count()-2);
    } else if (filename.endsWith("index.bdmv", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 2)
            filename = filenameParts.at(filenameParts.count()-3);
    }

    QRegExp rx("S(\\d+)[\\._\\-]?E", Qt::CaseInsensitive);
    if (rx.indexIn(filename) != -1)
        return rx.cap(1).toInt();
    rx.setPattern("(\\d+)?x(\\d+)");
    if (rx.indexIn(filename) != -1)
        return rx.cap(1).toInt();
    rx.setPattern("(\\d+)(\\d){2}");
    if (rx.indexIn(filename) != -1)
        return rx.cap(1).toInt();
    rx.setPattern("Season[._ ]?(\\d+)[._ ]?Episode");
    if (rx.indexIn(filename) != -1)
        return rx.cap(1).toInt();

    return 0;
}

QList<int> TvShowFileSearcher::getEpisodeNumbers(QStringList files)
{
    QList<int> episodes;
    if (files.isEmpty())
        return episodes;

    QStringList filenameParts = files.at(0).split(QDir::separator());
    QString filename = filenameParts.last();
    if (filename.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 1 && Helper::instance()->isDvd(files.at(0)))
            filename = filenameParts.at(filenameParts.count()-3);
        else if (filenameParts.count() > 2 && Helper::instance()->isDvd(files.at(0), true))
            filename = filenameParts.at(filenameParts.count()-2);
    } else if (filename.endsWith("index.bdmv", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 2)
            filename = filenameParts.at(filenameParts.count()-3);
    }

    QStringList patterns;
    patterns << "S(\\d+)[\\._\\-]?E(\\d+)" << "S(\\d+)EP(\\d+)" << "(\\d+)x(\\d+)" << "(\\d+)(\\d){2}" << "Season[._ ]?(\\d+)[._ ]?Episode[._ ]?(\\d+)";
    QRegExp rx;
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    foreach (const QString &pattern, patterns) {
        rx.setPattern(pattern);
        int pos = 0;
        int lastPos = -1;
        while ((pos = rx.indexIn(filename, pos)) != -1) {
            // if between the last match and this one are more than five characters: break
            // this way we can try to filter "false matches" like in "21x04 - Hammond vs. 6x6.mp4"
            if (lastPos != -1 && lastPos < pos+5)
                break;
            episodes << rx.cap(2).toInt();
            pos += rx.matchedLength();
            lastPos = pos;
        }
        pos = lastPos;

        // Pattern matched
        if (!episodes.isEmpty()) {
            if (episodes.count() == 1) {
                rx.setPattern("^[-_EeXx]+([0-9]+)($|[\\-\\._\\sE])");
                while (rx.indexIn(filename, pos, QRegExp::CaretAtOffset) != -1) {
                    episodes << rx.cap(1).toInt();
                    pos += rx.matchedLength()-1;
                }
            }
            break;
        }
    }

    return episodes;
}
