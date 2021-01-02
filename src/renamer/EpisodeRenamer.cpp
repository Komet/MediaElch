#include "renamer/EpisodeRenamer.h"

#include "globals/Helper.h"
#include "globals/Manager.h"
#include "media_centers/MediaCenterInterface.h"
#include "tv_shows/TvShowEpisode.h"

#include <QDir>
#include <QFileInfo>

EpisodeRenamer::EpisodeRenamer(RenamerConfig renamerConfig, RenamerDialog* dialog) : Renamer(renamerConfig, dialog)
{
}

EpisodeRenamer::RenameError EpisodeRenamer::renameEpisode(TvShowEpisode& episode,
    QVector<TvShowEpisode*>& episodesRenamed)
{
    const QString& seasonPattern = m_config.directoryPattern;
    const bool useSeasonDirectories = m_config.renameDirectories;

    bool errorOccured = false;

    QVector<TvShowEpisode*> multiEpisodes;
    for (TvShowEpisode* subEpisode : episode.tvShow()->episodes()) {
        if (subEpisode->files() == episode.files()) {
            multiEpisodes.append(subEpisode);
            episodesRenamed.append(subEpisode);
        }
    }

    const mediaelch::FilePath firstEpisode = episode.files().first();
    const bool isBluRay = helper::isBluRay(firstEpisode);
    const bool isDvd = helper::isDvd(firstEpisode);
    const bool isDvdWithoutSub = helper::isDvd(firstEpisode, true);

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
        for (const mediaelch::FilePath& file : episode.files()) {
            newFileName = (episode.files().count() == 1) ? m_config.filePattern : m_config.filePatternMulti;
            QFileInfo episodeFileInfo(file.toString());
            QString baseName = episodeFileInfo.completeBaseName();
            QDir currentDir = episodeFileInfo.dir();
            Renamer::replace(newFileName, "title", episode.title());
            Renamer::replace(newFileName, "showTitle", episode.showTitle());
            Renamer::replace(newFileName, "year", episode.firstAired().toString("yyyy"));
            Renamer::replace(newFileName, "extension", episodeFileInfo.suffix());
            Renamer::replace(newFileName, "season", episode.seasonString());
            Renamer::replace(newFileName, "partNo", QString::number(++partNo));
            Renamer::replace(newFileName, "videoCodec", episode.streamDetails()->videoCodec());
            Renamer::replace(newFileName, "audioCodec", episode.streamDetails()->audioCodec());
            Renamer::replace(newFileName, "channels", QString::number(episode.streamDetails()->audioChannels()));
            Renamer::replace(newFileName,
                "resolution",
                helper::matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::ScanType)));
            Renamer::replaceCondition(
                newFileName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");

            if (multiEpisodes.count() > 1) {
                QStringList episodeStrings;
                for (TvShowEpisode* subEpisode : multiEpisodes) {
                    episodeStrings.append(subEpisode->episodeString());
                }
                std::sort(episodeStrings.begin(), episodeStrings.end());
                Renamer::replace(newFileName, "episode", episodeStrings.join("-"));
            } else {
                Renamer::replace(newFileName, "episode", episode.episodeString());
            }

            helper::sanitizeFileName(newFileName);
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
                for (const QString& extra : m_extraFiles.filters()) {
                    filters << baseName + extra;
                }
                for (const QString& subFileName : currentDir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot)) {
                    QString subSuffix = subFileName.mid(baseName.length());
                    QString newBaseName = newFileName.left(newFileName.lastIndexOf("."));
                    QString newSubName = newBaseName + subSuffix;
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
                helper::sanitizeFileName(newNfoFileName);
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
                helper::sanitizeFileName(newThumbnailFileName);
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
        for (const QString& file : newEpisodeFiles) {
            files << episodeFileinfo.path() + "/" + file;
        }
        episode.setFiles(files);
        Manager::instance()->database()->update(&episode);
    }

    if (useSeasonDirectories) {
        QDir showDir(episode.tvShow()->dir().toString());
        QString seasonDirName = seasonPattern;
        Renamer::replace(seasonDirName, "season", episode.seasonString());
        Renamer::replace(seasonDirName, "showTitle", episode.showTitle());
        helper::sanitizeFolderName(seasonDirName);
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
                        Manager::instance()->database()->update(&episode);
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
                Manager::instance()->database()->update(&episode);
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
                        m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
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
