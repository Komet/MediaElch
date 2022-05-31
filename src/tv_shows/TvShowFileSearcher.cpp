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
            qCWarning(generic) << "[TvShowFileSearcher] TV show directory is excluded by advanced settings! "
                                  "Is this intended? Directory:"
                               << dir.path.path();
            continue;
        }

        if (!dir.path.isReadable()) {
            qCDebug(generic) << "[TvShowFileSearcher] TV show directory is not readable, skipping:" << dir.path.path();
            continue;
        }

        qCDebug(generic) << "[TvShowFileSearcher] Adding TV show directory" << dir.path.path();
        m_directories.append(dir);
    }
}

/// \brief Starts the scan process
void TvShowFileSearcher::reload(bool force)
{
    qCInfo(generic) << "[TvShowFileSearcher] Reload TV shows, clear database:" << force;
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

    qCDebug(generic) << "[TvShowFileSearcher] Searching for TV shows done";
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
    mediaelch::DirectoryPath path;
    elch_ssize_t index = -1;
    for (elch_ssize_t i = 0, n = m_directories.count(); i < n; ++i) {
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
        path = mediaelch::DirectoryPath(m_directories[index].path);
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
    int episodeSum = qsizetype_to_int(contents.count());

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

    QRegularExpression rx("((?:part|cd)[\\s_]*)(\\d+)", QRegularExpression::CaseInsensitiveOption);
    for (elch_ssize_t i = 0, n = files.size(); i < n; i++) {
        if (m_aborted) {
            return;
        }

        QStringList tvShowFiles;
        QString file = files.at(i);
        if (file.isEmpty()) {
            continue;
        }

        tvShowFiles << (path.toString() + '/' + file);

        QRegularExpressionMatch match = rx.match(file);
        elch_ssize_t pos = match.capturedStart(0);
        if (pos != -1) {
            QString left = file.left(pos) + match.captured(1);
            QString right = file.mid(pos + match.captured(1).size() + match.captured(2).size());
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

    QRegularExpression rx(R"(S(\d+)[ ._-]?E)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match;

    match = rx.match(filename);
    if (match.hasMatch()) {
        return SeasonNumber(match.captured(1).toInt());
    }

    rx.setPattern("(\\d+)?x(\\d+)");
    match = rx.match(filename);
    if (match.hasMatch()) {
        return SeasonNumber(match.captured(1).toInt());
    }

    rx.setPattern("(\\d+).(\\d){2,4}");
    match = rx.match(filename);
    if (match.hasMatch()) {
        return SeasonNumber(match.captured(1).toInt());
    }

    rx.setPattern("Season[ ._]?(\\d+)[ ._]?Episode");
    match = rx.match(filename);
    if (match.hasMatch()) {
        return SeasonNumber(match.captured(1).toInt());
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
        QRegularExpression rx(pattern, QRegularExpression::CaseInsensitiveOption);

        QRegularExpressionMatchIterator matches = rx.globalMatch(filename);

        elch_ssize_t lastMatchEnd = -1;
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            // if between the last match and this one are more than five characters: break
            // this way we can try to filter "false matches" like in "21x04 - Hammond vs. 6x6.mp4"
            if (mayBeAmbiguous && lastMatchEnd != -1 && lastMatchEnd < match.capturedStart(0) + 5) {
                return true;
            }
            episodes << EpisodeNumber(match.captured(2).toInt());
            lastMatchEnd = match.capturedEnd(0);
        }

        // Pattern did not match
        if (episodes.isEmpty()) {
            return false;
        }

        // The one episode we found could actually be a multi-episode file.
        // To avoid false positives, we use a positive lookahead.
        // For example: "S01E01E02E03 - Name.mov"
        if (episodes.count() == 1) {
            rx.setPattern(R"([-_EeXx]+(\d+)(?=$|[ -._sEeXx]))");

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            matches = rx.globalMatch(
                filename, lastMatchEnd, QRegularExpression::NormalMatch, QRegularExpression::AnchoredMatchOption);
#else
            matches = rx.globalMatch(
                filename, lastMatchEnd, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption);
#endif

            while (matches.hasNext()) {
                episodes << EpisodeNumber(matches.next().captured(1).toInt());
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

    for (const SettingsDir& dir : asConst(m_directories)) {
        if (dir.autoReload || dir.disabled) {
            database().clearTvShowsInDirectory(mediaelch::DirectoryPath(dir.path));
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
        episodeSum += qsizetype_to_int(it.value().size());
    }
    it.toFront();

    // Setup shows
    while (it.hasNext()) {
        if (m_aborted) {
            return;
        }

        it.next();

        // get path
        mediaelch::DirectoryPath path;
        elch_ssize_t index = -1;
        for (elch_ssize_t i = 0, n = m_directories.count(); i < n; ++i) {
            if (it.key().startsWith(m_directories[i].path.path())) {
                if (index == -1) {
                    index = i;
                } else if (m_directories[index].path.path().length() < m_directories[i].path.path().length()) {
                    index = i;
                }
            }
        }
        if (index != -1) {
            path = mediaelch::DirectoryPath(m_directories[index].path);
        }

        auto* show = new TvShow(mediaelch::DirectoryPath(it.key()), this);
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
        for (TvShowEpisode* episode : asConst(episodes)) {
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
    for (const SettingsDir& dir : asConst(m_directories)) {
        if (m_aborted) {
            break;
        }
        if (dir.disabled) {
            continue;
        }
        // Do we need to reload shows from disk?
        if (dir.autoReload || forceReload) {
            getTvShows(mediaelch::DirectoryPath(dir.path), contents);
            continue;
        }
        // TODO: Check if necessary?
        // If there are no shows in the database for the directory, reload
        // all shows regardless of forceReload.
        const int showsFromDatabase = database().showCount(mediaelch::DirectoryPath(dir.path));
        if (showsFromDatabase == 0) {
            getTvShows(mediaelch::DirectoryPath(dir.path), contents);
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
    for (const SettingsDir& dir : asConst(m_directories)) {
        if (dir.autoReload) { // Those directories are not read from database.
            continue;
        }
        if (dir.disabled) {
            continue;
        }
        QVector<TvShow*> showsFromDatabase = database().showsInDirectory(mediaelch::DirectoryPath(dir.path));
        if (!showsFromDatabase.isEmpty()) {
            dbShows.append(showsFromDatabase);
        }
    }
    return dbShows;
}
