#include "renamer/EpisodeRenamer.h"

#include "RenamerUtils.h"
#include "data/tv_show/TvShowEpisode.h"
#include "database/TvShowPersistence.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "log/Log.h"
#include "media_center/MediaCenterInterface.h"

#include <QDir>
#include <QFileInfo>

namespace mediaelch {

TvShowRenamerPlaceholders::~TvShowRenamerPlaceholders() = default;

QVector<Placeholder> TvShowRenamerPlaceholders::placeholders()
{
    return {
        // clang-format off
        { "title",            true,  false, QObject::tr("Title") },
        { "showTitle",        true,  false, QObject::tr("Show Title") },
        { "tmdbId",           true,  true,  QObject::tr("TMDB ID") },
        { "year",             true,  false, QObject::tr("Year") },
        // clang-format on
    };
}


EpisodeRenamerPlaceholders::~EpisodeRenamerPlaceholders() = default;

QVector<Placeholder> EpisodeRenamerPlaceholders::placeholders()
{
    return {
        // clang-format off
        { "extension",        true,  false, QObject::tr("File extension") },
        { "partNo",           true,  false, QObject::tr("Part number of the current file") },
        { "title",            true,  false, QObject::tr("Title") },
        { "episode",          true,  false, QObject::tr("Episode") },
        { "season",           true,  false, QObject::tr("Season") },
        { "seasonName",       true,  true,  QObject::tr("Season Name") },
        { "showTitle",        true,  false, QObject::tr("Show Title") },
        { "year",             true,  false, QObject::tr("Year") },
        { "3D",               false, true,  QObject::tr("File is 3D") },
        { "bluray",           false, true,  QObject::tr("File is BluRay") },
        { "dvd",              false, true,  QObject::tr("File is DVD") },
        { "audioCodec",       true,  false, QObject::tr("Audio Codec") },
        { "videoCodec",       true,  false, QObject::tr("Video Codec") },
        { "audioLanguage",    true,  false, QObject::tr("Audio Language(s) (separated by a minus)") },
        { "subtitleLanguage", true,  false, QObject::tr("Subtitle Language(s) (separated by a minus)") },
        { "channels",         true,  false, QObject::tr("Number of audio channels") },
        { "tmdbId",           true,  true,  QObject::tr("TMDB ID") },
        { "imdbId",           true,  true,  QObject::tr("IMDb ID") },
        { "resolution",       true,  false, QObject::tr("Resolution (1080p, 720p, ...)") },
        // clang-format on
    };
}

TvShowRenamerData::~TvShowRenamerData() = default;

ELCH_NODISCARD QString TvShowRenamerData::value(const QString& name) const
{
    const QMap<QString, std::function<QString()>> map = {
        // clang-format off
        {"title",            [this]() { return m_tvShow.title(); }},
        {"showTitle",        [this]() { return m_tvShow.title(); }},
        {"tmdbId",           [this]() { return m_tvShow.tmdbId().toString(); }},
        {"year",             [this]() { return m_tvShow.firstAired().toString("yyyy"); }},
        // clang-format on
    };

    MediaElch_Ensure_Data_Matches_Placeholders(TvShowRenamerPlaceholders, map);

    if (map.contains(name)) {
        return map[name]();
    }
    qCCritical(generic) << "TvShowRenamerData::value: Unknown tag:" << name;
    MediaElch_Debug_Unreachable();
    return "";
}

ELCH_NODISCARD bool TvShowRenamerData::passesCondition(const QString& name) const
{
    const QMap<QString, std::function<bool()>> map = {
        // no entry, yet
    };

    MediaElch_Ensure_Condition_Matches_Placeholders(TvShowRenamerPlaceholders, map);

    return map.contains(name) //
               ? map[name]()
               : !value(name).isEmpty();
}

EpisodeRenamerData::~EpisodeRenamerData() = default;

void EpisodeRenamerData::setMultiEpisodes(QVector<TvShowEpisode*> multiEpisodes)
{
    if (multiEpisodes.count() > 1) {
        QStringList episodeStrings;
        for (TvShowEpisode* subEpisode : multiEpisodes) {
            episodeStrings.append(subEpisode->episodeString());
        }
        std::sort(episodeStrings.begin(), episodeStrings.end());
        m_episodeString = episodeStrings.join("-");
    } else {
        m_episodeString = m_episode.episodeString();
    }
}

ELCH_NODISCARD QString EpisodeRenamerData::value(const QString& name) const
{
    const QMap<QString, std::function<QString()>> map = {
        // clang-format off
        {"extension",        [this]() { return m_extension; }},
        {"partNo",           [this]() { return QString::number(m_partNo); }},
        {"title",            [this]() { return m_episode.title(); }},
        {"episode",          [this]() { return m_episodeString; }},
        {"season",           [this]() { return m_episode.seasonString(); }},
        {"seasonName",       [this]() { return m_episode.seasonName(); }},
        {"showTitle",        [this]() { return m_episode.showTitle(); }},
        {"year",             [this]() { return m_episode.firstAired().toString("yyyy"); }},
        {"audioCodec",       [this]() { return m_episode.streamDetails()->audioCodec(); }},
        {"videoCodec",       [this]() { return m_episode.streamDetails()->videoCodec(); }},
        {"audioLanguage",    [this]() { return m_episode.streamDetails()->allAudioLanguages().join("-"); }},
        {"subtitleLanguage", [this]() { return m_episode.streamDetails()->allSubtitleLanguages().join("-"); }},
        {"channels",         [this]() { return QString::number(m_episode.streamDetails()->audioChannels()); }},
        {"imdbId",           [this]() { return m_episode.imdbId().toString(); }},
        {"tmdbId",           [this]() { return m_episode.tmdbId().toString(); }},
        {"resolution",       [this]() {
            return helper::matchResolution(m_videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                m_videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                m_videoDetails.value(StreamDetails::VideoDetails::ScanType));
        }},
        // clang-format on
    };

    MediaElch_Ensure_Data_Matches_Placeholders(EpisodeRenamerPlaceholders, map);

    if (map.contains(name)) {
        return map[name]();
    }
    qCCritical(generic) << "TvShowRenamerData::value: Unknown tag:" << name;
    MediaElch_Debug_Unreachable();
    return "";
}

ELCH_NODISCARD bool EpisodeRenamerData::passesCondition(const QString& name) const
{
    const QMap<QString, std::function<bool()>> map = {
        // clang-format off
        {"bluray", [this]() { return m_isBluRay; }},
        {"dvd",    [this]() { return m_isDvd; }},
        {"3D",     [this]() { return m_videoDetails.value(StreamDetails::VideoDetails::StereoMode) != ""; }},
        // clang-format on
    };

    MediaElch_Ensure_Condition_Matches_Placeholders(EpisodeRenamerPlaceholders, map);

    return map.contains(name) //
               ? map[name]()
               : !value(name).isEmpty();
}

} // namespace mediaelch


EpisodeRenamer::EpisodeRenamer(RenamerConfig renamerConfig, RenamerDialog* dialog) : Renamer(renamerConfig, dialog)
{
}

EpisodeRenamer::RenameError EpisodeRenamer::renameEpisode(TvShowEpisode& episode,
    QVector<TvShowEpisode*>& episodesRenamed)
{
    mediaelch::EpisodeRenamerPlaceholders renamerPlaceholder;
    mediaelch::EpisodeRenamerData renamerData{episode};

    mediaelch::TvShowPersistence persistence{*Manager::instance()->database()};

    const QString& seasonPattern = m_config.directoryPattern;
    const bool useSeasonDirectories = m_config.renameDirectories;

    // " ", i.e. space, is the default for sanitizeFileName().
    QString delimiter = (m_config.replaceDelimiter) ? m_config.delimiter : " ";

    bool errorOccured = false;

    QVector<TvShowEpisode*> multiEpisodes;
    for (TvShowEpisode* subEpisode : episode.tvShow()->episodes()) {
        if (subEpisode->files() == episode.files()) {
            multiEpisodes.append(subEpisode);
            episodesRenamed.append(subEpisode);
        }
    }
    renamerData.setMultiEpisodes(multiEpisodes);

    const mediaelch::FilePath firstEpisode = episode.files().first();
    const bool isBluRay = helper::isBluRay(firstEpisode);
    const bool isDvd = helper::isDvd(firstEpisode);
    const bool isDvdWithoutSub = helper::isDvd(firstEpisode, true);

    renamerData.setIsBluRay(isBluRay);
    renamerData.setIsDvd(isDvd);

    QFileInfo episodeFileinfo(episode.files().first().toString());
    QString fiCanonicalPath = episodeFileinfo.canonicalPath();
    mediaelch::FileList episodeFiles = episode.files();
    MediaCenterInterface* mediaCenter = Manager::instance()->mediaCenterInterface();
    QString nfo = mediaCenter->nfoFilePath(&episode);
    QString newNfoFileName = nfo;
    QString thumbnail = mediaCenter->imageFileName(&episode, ImageType::TvShowEpisodeThumb);
    QString newThumbnailFileName = thumbnail;
    QStringList newEpisodeFiles;

    for (const mediaelch::FilePath& file : episode.files()) {
        QFileInfo info(file.toString());
        newEpisodeFiles << info.fileName();
    }

    if (!isBluRay && !isDvd && !isDvdWithoutSub && m_config.renameFiles) {
        QString newFileName;
        episodeFiles.clear();

        newEpisodeFiles.clear();
        int partNo = 0;
        const auto videoDetails = episode.streamDetails()->videoDetails();
        renamerData.setVideoDetails(videoDetails);
        for (const mediaelch::FilePath& file : episode.files()) {
            newFileName = (episode.files().count() == 1) ? m_config.filePattern : m_config.filePatternMulti;
            QFileInfo episodeFileInfo(file.toString());
            QString baseName = episodeFileInfo.completeBaseName();
            QDir currentDir = episodeFileInfo.dir();

            renamerData.setPartNo(++partNo);
            renamerData.setExtension(episodeFileInfo.suffix());
            newFileName = renamerPlaceholder.replace(newFileName, renamerData);

            // Sanitize + Replace Delimiter with the one chosen by the user
            helper::sanitizeFileName(newFileName, delimiter);

            if (episodeFileInfo.fileName() != newFileName) {
                const int episodeRow = m_dialog->addResultToTable(
                    episodeFileInfo.fileName(), newFileName, Renamer::RenameOperation::Rename);
                if (!m_config.dryRun) {
                    if (!Renamer::rename(file.toString(), episodeFileInfo.canonicalPath() + "/" + newFileName)) {
                        m_dialog->setResultStatus(episodeRow, Renamer::RenameResult::Failed);
                        errorOccured = true;
                    } else {
                        newEpisodeFiles << newFileName;
                    }
                }

                QStringList filters;
                for (const QString& extra : m_extraFiles.fileGlob) {
                    filters << baseName + extra;
                }
                for (const QString& subFileName : currentDir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot)) {
                    QString subSuffix = subFileName.mid(baseName.length());
                    QString newBaseName = newFileName.left(newFileName.lastIndexOf("."));
                    QString newSubName = newBaseName + subSuffix;
                    // Replace Delimiter with the one chosen by the user
                    replaceDelimiter(newSubName);
                    const int row =
                        m_dialog->addResultToTable(subFileName, newSubName, Renamer::RenameOperation::Rename);
                    if (!m_config.dryRun) {
                        if (!Renamer::rename(currentDir.canonicalPath() + "/" + subFileName,
                                currentDir.canonicalPath() + "/" + newSubName)) {
                            m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
                            errorOccured = true;
                        }
                    }
                }
            } else {
                newEpisodeFiles << episodeFileInfo.fileName();
            }
            episodeFiles << mediaelch::FilePath(episodeFileInfo.canonicalPath() + "/" + newFileName);
        }

        // Rename nfo
        if (!nfo.isEmpty()) {
            QString nfoFileName = QFileInfo(nfo).fileName();
            QVector<DataFile> nfoFiles = Settings::instance()->dataFiles(DataFileType::TvShowEpisodeNfo);
            if (!nfoFiles.isEmpty()) {
                newNfoFileName = nfoFiles.first().saveFileName(newFileName);
                // Sanitize + Replace Delimiter with the one chosen by the user
                helper::sanitizeFileName(newNfoFileName, delimiter);
                if (newNfoFileName != nfoFileName) {
                    int row = m_dialog->addResultToTable(nfoFileName, newNfoFileName, Renamer::RenameOperation::Rename);
                    if (!m_config.dryRun) {
                        if (!Renamer::rename(nfo, fiCanonicalPath + "/" + newNfoFileName)) {
                            m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
                        }
                    }
                }
            }
        }

        // Rename Thumbnail
        if (!thumbnail.isEmpty()) {
            QString thumbnailFileName = QFileInfo(thumbnail).fileName();
            QVector<DataFile> thumbnailFiles = Settings::instance()->dataFiles(DataFileType::TvShowEpisodeThumb);
            if (!thumbnailFiles.isEmpty()) {
                newThumbnailFileName = thumbnailFiles.first().saveFileName(
                    newFileName, SeasonNumber::NoSeason, episode.files().count() > 1);
                // Sanitize + Replace Delimiter with the one chosen by the user
                helper::sanitizeFileName(newThumbnailFileName, delimiter);
                if (newThumbnailFileName != thumbnailFileName) {
                    int row = m_dialog->addResultToTable(
                        thumbnailFileName, newThumbnailFileName, Renamer::RenameOperation::Rename);
                    if (!m_config.dryRun) {
                        if (!Renamer::rename(thumbnail, fiCanonicalPath + "/" + newThumbnailFileName)) {
                            m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
                        }
                    }
                }
            }
        }
    }

    if (!m_config.dryRun) {
        QStringList files;
        for (const QString& file : asConst(newEpisodeFiles)) {
            files << episodeFileinfo.path() + "/" + file;
        }
        episode.setFiles(files);
        persistence.update(&episode);
    }

    if (useSeasonDirectories) {
        QDir showDir(episode.tvShow()->dir().toString());
        QString seasonDirName = seasonPattern;

        renamerData.setPartNo(0);
        renamerData.setExtension("");
        seasonDirName = renamerPlaceholder.replace(seasonDirName, renamerData);

        // Sanitize + Replace Delimiter with the one chosen by the user
        helper::sanitizeFolderName(seasonDirName, delimiter);

        QDir seasonDir(showDir.path() + "/" + seasonDirName);
        if (!seasonDir.exists()) {
            int row = m_dialog->addResultToTable(seasonDirName, "", Renamer::RenameOperation::CreateDir);
            if (!m_config.dryRun) {
                if (!showDir.mkdir(seasonDirName)) {
                    m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
                }
            }
        }

        if (isBluRay || isDvd || isDvdWithoutSub) {
            QDir dir = episodeFileinfo.dir();
            if (isDvd || isBluRay) {
                dir.cdUp();
            }

            QDir parentDir = dir;
            parentDir.cdUp();
            if (parentDir != seasonDir) {
                int row = m_dialog->addResultToTable(dir.dirName(), seasonDirName, Renamer::RenameOperation::Move);
                if (!m_config.dryRun) {
                    if (!Renamer::rename(dir, seasonDir.absolutePath() + "/" + dir.dirName())) {
                        m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
                        errorOccured = true;
                    } else {
                        newEpisodeFiles.clear();
                        QString oldDir = dir.absolutePath();
                        QString newDir = seasonDir.absolutePath() + "/" + dir.dirName();
                        for (const mediaelch::FilePath& file : episode.files()) {
                            newEpisodeFiles << newDir + file.toString().mid(oldDir.length());
                        }
                        episode.setFiles(newEpisodeFiles);
                        persistence.update(&episode);
                    }
                }
            }
        } else if (episodeFileinfo.dir() != seasonDir) {
            newEpisodeFiles.clear();
            for (const mediaelch::FilePath& fileName : episode.files()) {
                QFileInfo fi(fileName.toString());
                int row = m_dialog->addResultToTable(fi.fileName(), seasonDirName, Renamer::RenameOperation::Move);
                if (!m_config.dryRun) {
                    if (!Renamer::rename(fileName.toString(), seasonDir.path() + "/" + fi.fileName())) {
                        m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
                        errorOccured = true;
                    } else {
                        newEpisodeFiles << seasonDir.path() + "/" + fi.fileName();
                    }
                }
            }
            if (!m_config.dryRun) {
                episode.setFiles(newEpisodeFiles);
                persistence.update(&episode);
            }

            if (!newNfoFileName.isEmpty() && !nfo.isEmpty()) {
                int row = m_dialog->addResultToTable(newNfoFileName, seasonDirName, Renamer::RenameOperation::Move);
                if (!m_config.dryRun) {
                    if (!Renamer::rename(
                            fiCanonicalPath + "/" + newNfoFileName, seasonDir.path() + "/" + newNfoFileName)) {
                        m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
                    }
                }
            }
            if (!thumbnail.isEmpty() && !newThumbnailFileName.isEmpty()) {
                int row =
                    m_dialog->addResultToTable(newThumbnailFileName, seasonDirName, Renamer::RenameOperation::Move);
                if (!m_config.dryRun) {
                    if (!Renamer::rename(fiCanonicalPath + "/" + newThumbnailFileName,
                            seasonDir.path() + "/" + newThumbnailFileName)) {
                        m_dialog->setResultStatus(row, RenameResult::Failed);
                    }
                }
            }
        }
    }

    if (errorOccured) {
        return RenameError::Error;
    }
    return RenameError::None;
}

Renamer::RenameError EpisodeRenamer::renameTvShow(TvShow& tvShow)
{
    mediaelch::TvShowRenamerPlaceholders renamerPlaceholder;
    mediaelch::TvShowRenamerData renamerData{tvShow};

    mediaelch::TvShowPersistence persistence{*Manager::instance()->database()};

    QDir dir(tvShow.dir().toString());
    QString newFolderName = m_config.directoryPattern;

    newFolderName = renamerPlaceholder.replace(newFolderName, renamerData);

    helper::sanitizeFolderName(newFolderName);

    if (newFolderName != dir.dirName()) {
        const int row = m_dialog->addResultToTable(dir.dirName(), newFolderName, Renamer::RenameOperation::Rename);
        QDir parentDir(dir.path());
        parentDir.cdUp();
        if (m_config.dryRun) {
            return RenameError::None;
        }
        if (!Renamer::rename(dir, parentDir.absolutePath() + "/" + newFolderName)) {
            m_dialog->setResultStatus(row, RenameResult::Failed);
            return RenameError::Error;
        }
        const QString newShowDir = parentDir.absolutePath() + "/" + newFolderName;
        const QString oldShowDir = tvShow.dir().toString();
        tvShow.setDir(mediaelch::DirectoryPath(newShowDir));
        persistence.update(&tvShow);
        for (TvShowEpisode* episode : tvShow.episodes()) {
            QStringList files;
            for (const mediaelch::FilePath& file : episode->files()) {
                files << newShowDir + file.toString().mid(oldShowDir.length());
            }
            episode->setFiles(files);
            persistence.update(episode);
        }
    }

    return RenameError::None;
}
