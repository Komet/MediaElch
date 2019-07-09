#include "TvShowFileSearcher.h"

#include <QApplication>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QtConcurrent/QtConcurrentMap>

#include "globals/Helper.h"
#include "globals/Manager.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"
#include "tv_shows/model/EpisodeModelItem.h"
#include "tv_shows/model/SeasonModelItem.h"
#include "tv_shows/model/TvShowModelItem.h"

TvShowFileSearcher::TvShowFileSearcher(QObject* parent) :
    QObject(parent),
    m_progressMessageId{Constants::TvShowSearcherProgressMessageId},
    m_aborted{false}
{
}

void TvShowFileSearcher::setTvShowDirectories(QVector<SettingsDir> directories)
{
    m_directories.clear();
    for (auto& dir : directories) {
        if (dir.path.isReadable()) {
            qDebug() << "Adding tv show directory" << dir.path.path();
            m_directories.append(dir);
        } else {
            qDebug() << "Tv show directory is not redable, skipping:" << dir.path.path();
        }
    }
}

/// @brief Starts the scan process
void TvShowFileSearcher::reload(bool force)
{
    m_aborted = false;

    clearOldTvShows(force);

    emit searchStarted(tr("Searching for TV Shows..."));

    Manager::instance()->tvShowFilesWidget()->renewModel();

    QMap<QString, QVector<QStringList>> contents;
    for (SettingsDir dir : m_directories) {
        if (m_aborted) {
            return;
        }
        QString path = dir.path.path();
        QVector<TvShow*> showsFromDatabase = database().shows(path);
        if (dir.autoReload || force || showsFromDatabase.count() == 0) {
            getTvShows(path, contents);
        }
    }

    emit currentDir("");

    emit searchStarted(tr("Loading TV Shows..."));
    int episodeCounter = 0;
    int episodeSum = database().episodeCount();

    QVector<TvShow*> dbShows;
    for (SettingsDir dir : m_directories) {
        QVector<TvShow*> showsFromDatabase = database().shows(dir.path.path());
        if (!dir.autoReload && !force && !showsFromDatabase.isEmpty()) {
            dbShows.append(showsFromDatabase);
        }
    }

    setupShows(contents, episodeCounter, episodeSum);

    setupShowsFromDatabase(dbShows, episodeCounter, episodeSum);

    for (TvShow* show : Manager::instance()->tvShowModel()->tvShows()) {
        if (show->showMissingEpisodes()) {
            show->fillMissingEpisodes();
        }
    }

    qDebug() << "Searching for TV shows done";
    if (!m_aborted) {
        emit tvShowsLoaded();
    }
}

TvShowEpisode* TvShowFileSearcher::loadEpisodeData(TvShowEpisode* episode)
{
    episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow(), false);
    return episode;
}

void TvShowFileSearcher::reloadEpisodes(QString showDir)
{
    database().clearTvShow(showDir);
    emit searchStarted(tr("Searching for Episodes..."));

    // remove old show object
    for (TvShow* s : Manager::instance()->tvShowModel()->tvShows()) {
        if (m_aborted) {
            return;
        }

        if (s->dir() == showDir) {
            Manager::instance()->tvShowModel()->removeShow(s);
            break;
        }
    }

    // get path
    QString path;
    int index = -1;
    for (int i = 0, n = m_directories.count(); i < n; ++i) {
        if (m_aborted) {
            return;
        }

        if (showDir.startsWith(m_directories[i].path.path())) {
            if (index == -1) {
                index = i;
            } else if (m_directories[index].path.path().length() < m_directories[i].path.path().length()) {
                index = i;
            }
        }
    }
    if (index != -1) {
        path = m_directories[index].path.path();
    }

    // search for contents
    QVector<QStringList> contents;
    scanTvShowDir(path, showDir, contents);
    TvShow* show = new TvShow(showDir, this);
    show->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
    database().add(show, path);
    TvShowModelItem* showItem = Manager::instance()->tvShowModel()->appendChild(show);

    emit searchStarted(tr("Loading Episodes..."));
    emit currentDir(show->name());

    int episodeCounter = 0;
    int episodeSum = contents.count();
    QMap<SeasonNumber, SeasonModelItem*> seasonItems;
    QVector<TvShowEpisode*> episodes;
    for (const QStringList& files : contents) {
        if (m_aborted) {
            return;
        }
        SeasonNumber seasonNumber = getSeasonNumber(files);
        QVector<EpisodeNumber> episodeNumbers = getEpisodeNumbers(files);
        for (const EpisodeNumber& episodeNumber : episodeNumbers) {
            TvShowEpisode* episode = new TvShowEpisode(files, show);
            episode->setSeason(seasonNumber);
            episode->setEpisode(episodeNumber);
            episodes.append(episode);
        }
    }

    QtConcurrent::blockingMapped(episodes, TvShowFileSearcher::reloadEpisodeData);

    for (TvShowEpisode* episode : episodes) {
        database().add(episode, path, show->databaseId());
        show->addEpisode(episode);
        if (!seasonItems.contains(episode->season())) {
            seasonItems.insert(
                episode->season(), showItem->appendSeason(episode->season(), episode->seasonString(), show));
        }
        seasonItems.value(episode->season())->appendEpisode(episode);
        emit progress(++episodeCounter, episodeSum, m_progressMessageId);
        qApp->processEvents();
    }

    emit tvShowsLoaded();
}

TvShowEpisode* TvShowFileSearcher::reloadEpisodeData(TvShowEpisode* episode)
{
    episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
    return episode;
}

/**
 * @brief Scans a dir for tv show files
 * @param path Directory to scan
 * @param contents
 */
void TvShowFileSearcher::getTvShows(QString path, QMap<QString, QVector<QStringList>>& contents)
{
    QDir dir(path);
    QStringList tvShows = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& cDir : tvShows) {
        if (m_aborted) {
            return;
        }

        QVector<QStringList> tvShowContents;
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
void TvShowFileSearcher::scanTvShowDir(QString startPath, QString path, QVector<QStringList>& contents)
{
    emit currentDir(path.mid(startPath.length()));

    QDir dir(path);
    for (const QString& cDir : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (m_aborted) {
            return;
        }

        // Skip "Extras" folder
        if (QString::compare(cDir, "Extras", Qt::CaseInsensitive) == 0
            || QString::compare(cDir, ".actors", Qt::CaseInsensitive) == 0
            || QString::compare(cDir, "extrafanarts", Qt::CaseInsensitive) == 0) {
            continue;
        }

        // Handle DVD
        if (Helper::isDvd(path + QDir::separator() + cDir)) {
            contents.append(QStringList() << QDir::toNativeSeparators(path + "/" + cDir + "/VIDEO_TS/VIDEO_TS.IFO"));
            continue;
        }
        if (Helper::isDvd(path + QDir::separator() + cDir, true)) {
            contents.append(QStringList() << QDir::toNativeSeparators(path + "/" + cDir + "/VIDEO_TS.IFO"));
            continue;
        }

        // Handle BluRay
        if (Helper::isBluRay(path + QDir::separator() + cDir)) {
            contents.append(QStringList() << QDir::toNativeSeparators(path + "/" + cDir + "/BDMV/index.bdmv"));
            continue;
        }
        scanTvShowDir(startPath, path + "/" + cDir, contents);
    }

    QStringList files;
    QStringList entries = getFiles(path);
    for (const QString& file : entries) {
        // Skip Trailers and Sample files
        if (file.contains("-trailer", Qt::CaseInsensitive) || file.contains("-sample", Qt::CaseInsensitive)) {
            continue;
        }
        files.append(file);
    }
    files.sort();

    QRegExp rx("((part|cd)[\\s_]*)(\\d+)", Qt::CaseInsensitive);
    for (int i = 0, n = files.size(); i < n; i++) {
        if (m_aborted) {
            return;
        }

        QStringList tvShowFiles;
        QString file = files.at(i);
        if (file.isEmpty()) {
            continue;
        }

        tvShowFiles << QDir::toNativeSeparators(path + QDir::separator() + file);

        int pos = rx.indexIn(file);
        if (pos != -1) {
            QString left = file.left(pos) + rx.cap(1);
            QString right = file.mid(pos + rx.cap(1).size() + rx.cap(2).size());
            for (int x = 0; x < n; x++) {
                QString subFile = files.at(x);
                if (subFile != file) {
                    if (subFile.startsWith(left) && subFile.endsWith(right)) {
                        tvShowFiles << QDir::toNativeSeparators(path + QDir::separator() + subFile);
                        files[x] = ""; // set an empty file name, this way we can skip this file in the main loop
                    }
                }
            }
        }
        if (tvShowFiles.count() > 0) {
            contents.append(tvShowFiles);
        }
    }
}

/// Get a list of files in a directory
QStringList TvShowFileSearcher::getFiles(QString path)
{
    return Settings::instance()->advanced()->tvShowFilters().files(QDir(path));
}

void TvShowFileSearcher::abort()
{
    m_aborted = true;
}

SeasonNumber TvShowFileSearcher::getSeasonNumber(QStringList files)
{
    if (files.isEmpty()) {
        return SeasonNumber::NoSeason;
    }

    QStringList filenameParts = files.at(0).split(QDir::separator());
    QString filename = filenameParts.last();
    if (filename.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 1 && Helper::isDvd(files.at(0))) {
            filename = filenameParts.at(filenameParts.count() - 3);
        } else if (filenameParts.count() > 2 && Helper::isDvd(files.at(0), true)) {
            filename = filenameParts.at(filenameParts.count() - 2);
        }
    } else if (filename.endsWith("index.bdmv", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 2) {
            filename = filenameParts.at(filenameParts.count() - 3);
        }
    }

    QRegExp rx(R"(S(\d+)[\._\-]?E)", Qt::CaseInsensitive);
    if (rx.indexIn(filename) != -1) {
        return SeasonNumber(rx.cap(1).toInt());
    }
    rx.setPattern("(\\d+)?x(\\d+)");
    if (rx.indexIn(filename) != -1) {
        return SeasonNumber(rx.cap(1).toInt());
    }
    rx.setPattern("(\\d+)(\\d){2}");
    if (rx.indexIn(filename) != -1) {
        return SeasonNumber(rx.cap(1).toInt());
    }
    rx.setPattern("Season[._ ]?(\\d+)[._ ]?Episode");
    if (rx.indexIn(filename) != -1) {
        return SeasonNumber(rx.cap(1).toInt());
    }

    // Default if no valid season could be parsed.
    return SeasonNumber::SpecialsSeason;
}

QVector<EpisodeNumber> TvShowFileSearcher::getEpisodeNumbers(QStringList files)
{
    QVector<EpisodeNumber> episodes;
    if (files.isEmpty()) {
        return episodes;
    }

    QStringList filenameParts = files.at(0).split(QDir::separator());
    QString filename = filenameParts.last();
    if (filename.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 1 && Helper::isDvd(files.at(0))) {
            filename = filenameParts.at(filenameParts.count() - 3);
        } else if (filenameParts.count() > 2 && Helper::isDvd(files.at(0), true)) {
            filename = filenameParts.at(filenameParts.count() - 2);
        }
    } else if (filename.endsWith("index.bdmv", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 2) {
            filename = filenameParts.at(filenameParts.count() - 3);
        }
    }

    QStringList patterns;
    patterns << R"(S(\d+)[\._\-]?E(\d+))"
             << "S(\\d+)EP(\\d+)"
             << "(\\d+)x(\\d+)"
             << "(\\d+)(\\d){2}"
             << "Season[._ ]?(\\d+)[._ ]?Episode[._ ]?(\\d+)";
    QRegExp rx;
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    for (const QString& pattern : patterns) {
        rx.setPattern(pattern);
        int pos = 0;
        int lastPos = -1;
        while ((pos = rx.indexIn(filename, pos)) != -1) {
            // if between the last match and this one are more than five characters: break
            // this way we can try to filter "false matches" like in "21x04 - Hammond vs. 6x6.mp4"
            if (lastPos != -1 && lastPos < pos + 5) {
                break;
            }
            episodes << EpisodeNumber(rx.cap(2).toInt());
            pos += rx.matchedLength();
            lastPos = pos;
        }
        pos = lastPos;

        // Pattern matched
        if (!episodes.isEmpty()) {
            if (episodes.count() == 1) {
                rx.setPattern(R"(^[-_EeXx]+([0-9]+)($|[\-\._\sE]))");
                while (rx.indexIn(filename, pos, QRegExp::CaretAtOffset) != -1) {
                    episodes << EpisodeNumber(rx.cap(1).toInt());
                    pos += rx.matchedLength() - 1;
                }
            }
            break;
        }
    }

    return episodes;
}

Database& TvShowFileSearcher::database()
{
    return *Manager::instance()->database();
}

void TvShowFileSearcher::clearOldTvShows(bool forceClear)
{
    if (forceClear) {
        database().clearTvShows();
    }

    // clear gui
    Manager::instance()->tvShowModel()->clear();

    for (SettingsDir dir : m_directories) {
        if (dir.autoReload || forceClear) {
            database().clearTvShows(dir.path.path());
        }
    }
}

void TvShowFileSearcher::setupShowsFromDatabase(QVector<TvShow*>& dbShows, int episodeCounter, int episodeSum)
{
    for (TvShow* show : dbShows) {
        if (m_aborted) {
            return;
        }

        show->loadData(Manager::instance()->mediaCenterInterfaceTvShow(), false);
        TvShowModelItem* showItem = Manager::instance()->tvShowModel()->appendChild(show);

        QMap<SeasonNumber, SeasonModelItem*> seasonItems;
        QVector<TvShowEpisode*> episodes = database().episodes(show->databaseId());
        QtConcurrent::blockingMapped(episodes, TvShowFileSearcher::loadEpisodeData);
        for (TvShowEpisode* episode : episodes) {
            episode->setShow(show);
            show->addEpisode(episode);
            if (!seasonItems.contains(episode->season())) {
                seasonItems.insert(
                    episode->season(), showItem->appendSeason(episode->season(), episode->seasonString(), show));
            }
            seasonItems.value(episode->season())->appendEpisode(episode);
            emit progress(++episodeCounter, episodeSum, m_progressMessageId);
            if (episodeCounter % 1000 == 0) {
                emit currentDir("");
            }
        }
    }
}

void TvShowFileSearcher::setupShows(QMap<QString, QVector<QStringList>>& contents, int& episodeCounter, int episodeSum)
{
    QMapIterator<QString, QVector<QStringList>> it(contents);
    while (it.hasNext()) {
        it.next();
        episodeSum += it.value().size();
    }
    it.toFront();

    // Setup shows
    while (it.hasNext()) {
        if (m_aborted) {
            return;
        }

        it.next();

        // get path
        QString path;
        int index = -1;
        for (int i = 0, n = m_directories.count(); i < n; ++i) {
            if (it.key().startsWith(m_directories[i].path.path())) {
                if (index == -1) {
                    index = i;
                } else if (m_directories[index].path.path().length() < m_directories[i].path.path().length()) {
                    index = i;
                }
            }
        }
        if (index != -1) {
            path = m_directories[index].path.path();
        }

        TvShow* show = new TvShow(it.key(), this);
        show->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
        emit currentDir(show->name());
        database().add(show, path);
        TvShowModelItem* showItem = Manager::instance()->tvShowModel()->appendChild(show);

        database().transaction();
        QMap<SeasonNumber, SeasonModelItem*> seasonItems;
        QVector<TvShowEpisode*> episodes;

        // Setup episodes list
        for (const QStringList& files : it.value()) {
            SeasonNumber seasonNumber = getSeasonNumber(files);
            QVector<EpisodeNumber> episodeNumbers = getEpisodeNumbers(files);
            for (const EpisodeNumber& episodeNumber : episodeNumbers) {
                TvShowEpisode* episode = new TvShowEpisode(files, show);
                episode->setSeason(seasonNumber);
                episode->setEpisode(episodeNumber);
                episodes.append(episode);
            }
        }

        // Load episodes data
        QtConcurrent::blockingMapped(episodes, TvShowFileSearcher::reloadEpisodeData);

        // Add episodes to model
        for (TvShowEpisode* episode : episodes) {
            database().add(episode, path, show->databaseId());
            show->addEpisode(episode);
            if (!seasonItems.contains(episode->season())) {
                seasonItems.insert(
                    episode->season(), showItem->appendSeason(episode->season(), episode->seasonString(), show));
            }
            seasonItems.value(episode->season())->appendEpisode(episode);
            emit progress(++episodeCounter, episodeSum, m_progressMessageId);
        }

        database().commit();
    }

    emit currentDir("");
}
