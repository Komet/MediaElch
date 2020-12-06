#include "TvShowFileSearcher.h"

#include <QApplication>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QtConcurrent/QtConcurrentMap>

#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"
#include "tv_shows/model/EpisodeModelItem.h"
#include "tv_shows/model/SeasonModelItem.h"
#include "tv_shows/model/TvShowModelItem.h"

TvShowFileSearcher::TvShowFileSearcher(QObject* parent) :
    QObject(parent), m_progressMessageId{Constants::TvShowSearcherProgressMessageId}, m_aborted{false}
{
}

void TvShowFileSearcher::setTvShowDirectories(QVector<SettingsDir> directories)
{
    m_directories.clear();
    for (auto& dir : directories) {
        if (Settings::instance()->advanced()->isFolderExcluded(dir.path.dirName())) {
            qWarning() << "[TvShowFileSearcher] TV show directory is excluded by advanced settings! "
                          "Is this intended? Directory:"
                       << dir.path.path();
            continue;
        }

        if (!dir.path.isReadable()) {
            qDebug() << "[TvShowFileSearcher] TV show directory is not readable, skipping:" << dir.path.path();
            continue;
        }

        qDebug() << "[TvShowFileSearcher] Adding TV show directory" << dir.path.path();
        m_directories.append(dir);
    }
}

/// \brief Starts the scan process
void TvShowFileSearcher::reload(bool force)
{
    qInfo() << "[TvShowFileSearcher] Reload TV shows, clear database:" << force;
    m_aborted = false;

    clearOldTvShows(force);

    emit searchStarted(tr("Searching for TV Shows..."));

    auto files = readTvShowContent(force);

    emit currentDir("");

    emit searchStarted(tr("Loading TV Shows..."));
    int episodeCounter = 0;
    const int episodeSum = database().episodeCount();

    QVector<TvShow*> dbShows = getShowsFromDatabase(force);
    setupShows(files, episodeCounter, episodeSum);
    setupShowsFromDatabase(dbShows, episodeCounter, episodeSum);

    for (TvShow* show : Manager::instance()->tvShowModel()->tvShows()) {
        if (show->showMissingEpisodes()) {
            show->fillMissingEpisodes();
        }
    }

    qDebug() << "[TvShowFileSearcher] Searching for TV shows done";
    if (!m_aborted) {
        emit tvShowsLoaded();
    }
}

TvShowEpisode* TvShowFileSearcher::loadEpisodeData(TvShowEpisode* episode)
{
    episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow(), false, false);
    return episode;
}

void TvShowFileSearcher::reloadEpisodes(const mediaelch::DirectoryPath& showDir)
{
    database().clearTvShowInDirectory(showDir);
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

        if (showDir.toString().startsWith(m_directories[i].path.path())) {
            if (index == -1 || m_directories[index].path.path().length() < m_directories[i].path.path().length()) {
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
    auto* show = new TvShow(showDir, this);
    show->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
    database().add(show, path);

    emit searchStarted(tr("Loading Episodes..."));
    emit currentDir(show->title());

    int episodeCounter = 0;
    int episodeSum = contents.count();

    QVector<TvShowEpisode*> episodes;
    for (const QStringList& files : contents) {
        if (m_aborted) {
            return;
        }
        SeasonNumber seasonNumber = getSeasonNumber(files);
        QVector<EpisodeNumber> episodeNumbers = getEpisodeNumbers(files);
        for (const EpisodeNumber& episodeNumber : episodeNumbers) {
            auto* episode = new TvShowEpisode(files, show);
            episode->setSeason(seasonNumber);
            episode->setEpisode(episodeNumber);
            episodes.append(episode);
        }
    }

    QtConcurrent::blockingMapped(episodes, TvShowFileSearcher::reloadEpisodeData);

    for (TvShowEpisode* episode : episodes) {
        database().add(episode, path, show->databaseId());
        show->addEpisode(episode);
        emit progress(++episodeCounter, episodeSum, m_progressMessageId);
        QApplication::processEvents();
    }

    Manager::instance()->tvShowModel()->appendShow(show);

    emit tvShowsLoaded();
}

TvShowEpisode* TvShowFileSearcher::reloadEpisodeData(TvShowEpisode* episode)
{
    episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow(), true, true);
    return episode;
}

/**
 * \brief Scans a dir for TV show files
 * \param path Directory to scan
 */
void TvShowFileSearcher::getTvShows(const mediaelch::DirectoryPath& path, QMap<QString, QVector<QStringList>>& contents)
{
    QDir dir(path.toString());
    QStringList tvShows = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& cDir : tvShows) {
        if (m_aborted) {
            return;
        }

        if (Settings::instance()->advanced()->isFolderExcluded(cDir)) {
            continue;
        }

        QVector<QStringList> tvShowContents;
        scanTvShowDir(path, path.subDir(cDir), tvShowContents);
        contents.insert((dir.path() + '/' + cDir), tvShowContents);
    }
}

/**
 * \brief Scans the given path for TV show files.
 * Results are in a list which contains a QStringList for every episode.
 * \param startPath Scanning started at this path
 * \param path Path to scan
 * \param contents List of contents
 */
void TvShowFileSearcher::scanTvShowDir(const mediaelch::DirectoryPath& startPath,
    const mediaelch::DirectoryPath& path,
    QVector<QStringList>& contents)
{
    emit currentDir(path.toString().mid(startPath.toString().length()));

    QDir dir(path.toString());
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
            || QString::compare(cDir, "extrafanarts", Qt::CaseInsensitive) == 0) {
            continue;
        }

        // Handle DVD
        if (helper::isDvd(path.subDir(cDir))) {
            contents.append(QStringList() << (path.toString() + "/" + cDir + "/VIDEO_TS/VIDEO_TS.IFO"));
            continue;
        }
        if (helper::isDvd(path.subDir(cDir), true)) {
            contents.append(QStringList() << (path.toString() + "/" + cDir + "/VIDEO_TS.IFO"));
            continue;
        }

        // Handle BluRay
        if (helper::isBluRay(path.subDir(cDir))) {
            contents.append(QStringList() << (path.toString() + "/" + cDir + "/BDMV/index.bdmv"));
            continue;
        }
        scanTvShowDir(startPath, path.subDir(cDir), contents);
    }

    QStringList files;
    QStringList entries = getFiles(path);
    for (const QString& file : entries) {
        if (Settings::instance()->advanced()->isFileExcluded(file)) {
            continue;
        }
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

        tvShowFiles << (path.toString() + '/' + file);

        int pos = rx.indexIn(file);
        if (pos != -1) {
            QString left = file.left(pos) + rx.cap(1);
            QString right = file.mid(pos + rx.cap(1).size() + rx.cap(2).size());
            for (int x = 0; x < n; x++) {
                QString subFile = files.at(x);
                if (subFile != file) {
                    if (subFile.startsWith(left) && subFile.endsWith(right)) {
                        tvShowFiles << (path.toString() + '/' + subFile);
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
QStringList TvShowFileSearcher::getFiles(const mediaelch::DirectoryPath& path)
{
    return Settings::instance()->advanced()->tvShowFilters().files(QDir(path.toString()));
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

    QStringList filenameParts = files.at(0).split('/');
    QString filename = filenameParts.last();
    if (filename.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 1 && helper::isDvd(files.at(0))) {
            filename = filenameParts.at(filenameParts.count() - 3);
        } else if (filenameParts.count() > 2 && helper::isDvd(files.at(0), true)) {
            filename = filenameParts.at(filenameParts.count() - 2);
        }
    } else if (filename.endsWith("index.bdmv", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 2) {
            filename = filenameParts.at(filenameParts.count() - 3);
        }
    }

    QRegExp rx(R"(S(\d+)[ ._-]?E)", Qt::CaseInsensitive);
    if (rx.indexIn(filename) != -1) {
        return SeasonNumber(rx.cap(1).toInt());
    }
    rx.setPattern("(\\d+)?x(\\d+)");
    if (rx.indexIn(filename) != -1) {
        return SeasonNumber(rx.cap(1).toInt());
    }
    rx.setPattern("(\\d+).(\\d){2,4}");
    if (rx.indexIn(filename) != -1) {
        return SeasonNumber(rx.cap(1).toInt());
    }
    rx.setPattern("Season[ ._]?(\\d+)[ ._]?Episode");
    if (rx.indexIn(filename) != -1) {
        return SeasonNumber(rx.cap(1).toInt());
    }

    // Default if no valid season could be parsed.
    return SeasonNumber::SpecialsSeason;
}

QVector<EpisodeNumber> TvShowFileSearcher::getEpisodeNumbers(QStringList files)
{
    if (files.isEmpty()) {
        return {};
    }

    QStringList filenameParts = files.at(0).split('/');
    QString filename = filenameParts.last();
    if (filename.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 1 && helper::isDvd(files.at(0))) {
            filename = filenameParts.at(filenameParts.count() - 3);
        } else if (filenameParts.count() > 2 && helper::isDvd(files.at(0), true)) {
            filename = filenameParts.at(filenameParts.count() - 2);
        }
    } else if (filename.endsWith("index.bdmv", Qt::CaseInsensitive)) {
        if (filenameParts.count() > 2) {
            filename = filenameParts.at(filenameParts.count() - 3);
        }
    }


    QVector<EpisodeNumber> episodes;

    /// Scans a given filename for a given pattern.
    /// If mayBeAmbiguous is true, we apply a heuristic to avoid matching the video's resolution
    auto scanWithPattern = [&](const QString& pattern, bool mayBeAmbiguous) -> bool {
        QRegExp rx(pattern);
        rx.setCaseSensitivity(Qt::CaseInsensitive);

        int pos = 0;
        int lastPos = -1;
        while ((pos = rx.indexIn(filename, pos)) != -1) {
            // if between the last match and this one are more than five characters: break
            // this way we can try to filter "false matches" like in "21x04 - Hammond vs. 6x6.mp4"
            if (mayBeAmbiguous && lastPos != -1 && lastPos < pos + 5) {
                return true;
            }
            episodes << EpisodeNumber(rx.cap(2).toInt());
            pos += rx.matchedLength();
            lastPos = pos;
        }
        pos = lastPos;

        // Pattern matched
        if (episodes.isEmpty()) {
            return false;
        }

        if (episodes.count() == 1) {
            rx.setPattern(R"(^[-_EeXx]+([0-9]+)($|[\-\._\sE]))");
            while (rx.indexIn(filename, pos, QRegExp::CaretAtOffset) != -1) {
                episodes << EpisodeNumber(rx.cap(1).toInt());
                pos += rx.matchedLength() - 1;
            }
        }
        return true;
    };

    struct EpisodeNumberPattern
    {
        QString regex;
        bool mayBeAmbiguous = false;
    };

    QVector<EpisodeNumberPattern> patterns{{R"(S(\d+)[ ._-]?E(\d+))", false},
        {R"(S(\d+)[ ._-]?EP(\d+))", false},
        {R"(Season[ ._-]?(\d+)[._ -]?Episode[ ._-]?(\d+))", false},
        {R"((\d+)x(\d+))", true},
        {R"((\d+).(\d){2,4})", true}};

    for (const auto& pattern : patterns) {
        if (scanWithPattern(pattern.regex, pattern.mayBeAmbiguous)) {
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
    // clear gui
    Manager::instance()->tvShowModel()->clear();

    if (forceClear) {
        // Simply delete all shows
        database().clearAllTvShows();
        return;
    }

    for (const SettingsDir& dir : m_directories) {
        if (dir.autoReload) {
            database().clearTvShowsInDirectory(dir.path);
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

        QVector<TvShowEpisode*> episodes = database().episodes(show->databaseId());
        QtConcurrent::blockingMapped(episodes, TvShowFileSearcher::loadEpisodeData);
        for (TvShowEpisode* episode : episodes) {
            if (episode == nullptr) {
                continue;
            }
            episode->setShow(show);
            show->addEpisode(episode);
            emit progress(++episodeCounter, episodeSum, m_progressMessageId);
            if (episodeCounter % 1000 == 0) {
                emit currentDir("");
            }
        }

        Manager::instance()->tvShowModel()->appendShow(show);
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

        auto* show = new TvShow(it.key(), this);
        show->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
        emit currentDir(show->title());
        database().add(show, path);

        database().transaction();
        QVector<TvShowEpisode*> episodes;

        // Setup episodes list
        for (const QStringList& files : it.value()) {
            SeasonNumber seasonNumber = getSeasonNumber(files);
            QVector<EpisodeNumber> episodeNumbers = getEpisodeNumbers(files);
            for (const EpisodeNumber& episodeNumber : episodeNumbers) {
                auto* episode = new TvShowEpisode(files, show);
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
            emit progress(++episodeCounter, episodeSum, m_progressMessageId);
        }

        database().commit();
        Manager::instance()->tvShowModel()->appendShow(show);
    }

    emit currentDir("");
}

QMap<QString, QVector<QStringList>> TvShowFileSearcher::readTvShowContent(bool forceReload)
{
    QMap<QString, QVector<QStringList>> contents;
    for (const SettingsDir& dir : m_directories) {
        if (m_aborted) {
            break;
        }
        // Do we need to reload shows from disk?
        if (dir.autoReload || forceReload) {
            getTvShows(dir.path, contents);
            continue;
        }
        // TODO: Check if necessary?
        // If there are no shows in the database for the directory, reload
        // all shows regardless of forceReload.
        const int showsFromDatabase = database().showCount(dir.path);
        if (showsFromDatabase == 0) {
            getTvShows(dir.path, contents);
            continue;
        }
    }
    return contents;
}


QVector<TvShow*> TvShowFileSearcher::getShowsFromDatabase(bool forceReload)
{
    if (forceReload) {
        return {};
    }

    QVector<TvShow*> dbShows;
    for (const SettingsDir& dir : m_directories) {
        if (dir.autoReload) { // Those directories are not read from database.
            continue;
        }
        QVector<TvShow*> showsFromDatabase = database().showsInDirectory(dir.path);
        if (!showsFromDatabase.isEmpty()) {
            dbShows.append(showsFromDatabase);
        }
    }
    return dbShows;
}
