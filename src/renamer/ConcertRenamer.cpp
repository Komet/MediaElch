#include "ConcertRenamer.h"

#include "RenamerUtils.h"
#include "data/concert/Concert.h"
#include "database/ConcertPersistence.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "log/Log.h"
#include "media_center/MediaCenterInterface.h"

#include <QDir>
#include <QFileInfo>

namespace mediaelch {

ConcertRenamerPlaceholders::~ConcertRenamerPlaceholders() = default;

QVector<Placeholder> ConcertRenamerPlaceholders::placeholders()
{
    return {
        // clang-format off
        { "extension",        true,  false, QObject::tr("File extension") },
        { "partNo",           true,  false, QObject::tr("Part number of the current file") },
        { "title",            true,  false, QObject::tr("Title") },
        { "year",             true,  false, QObject::tr("Year") },
        { "audioCodec",       true,  false, QObject::tr("Audio Codec") },
        { "artist",           true,  false, QObject::tr("Artist") },
        { "album",           true,  false, QObject::tr("Album") },
        { "resolution",       true,  false, QObject::tr("Resolution (1080p, 720p, ...)") },
        { "channels",         true,  false, QObject::tr("Number of audio channels") },
        { "subtitleLanguage", true,  false, QObject::tr("Subtitle Language(s) (separated by a minus)") },
        { "videoCodec",       true,  false, QObject::tr("Video Codec") },
        { "audioLanguage",    true,  false, QObject::tr("Audio Language(s) (separated by a minus)") },
        { "3D",               false, true,  QObject::tr("File is 3D") },
        { "bluray",           false, true,  QObject::tr("File/directory is BluRay") },
        { "dvd",              false, true,  QObject::tr("File/directory is DVD") },
        // clang-format on
    };
}

ConcertRenamerData::~ConcertRenamerData() = default;

ELCH_NODISCARD QString ConcertRenamerData::value(const QString& name) const
{
    const QMap<QString, std::function<QString()>> map = {
        // clang-format off
        {"extension",        [this]() { return m_extension; }},
        {"artist",           [this]() { return m_concert.artists().isEmpty() ? "" : m_concert.artists().first(); }},
        {"album",            [this]() { return m_concert.album(); }},
        {"partNo",           [this]() { return QString::number(m_partNo); }},
        {"title",            [this]() { return m_concert.title(); }},
        {"year",             [this]() { return m_concert.released().toString("yyyy"); }},
        {"videoCodec",       [this]() { return m_concert.streamDetails()->videoCodec(); }},
        {"audioLanguage",    [this]() { return m_concert.streamDetails()->allAudioLanguages().join("-"); }},
        {"subtitleLanguage", [this]() { return m_concert.streamDetails()->allSubtitleLanguages().join("-"); }},
        {"audioCodec",       [this]() { return m_concert.streamDetails()->audioCodec(); }},
        {"channels",         [this]() { return QString::number(m_concert.streamDetails()->audioChannels()); }},
        {"resolution",       [this]() {
            return helper::matchResolution(m_videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                m_videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                m_videoDetails.value(StreamDetails::VideoDetails::ScanType));
        }},
        // clang-format on
    };

    MediaElch_Ensure_Data_Matches_Placeholders(ConcertRenamerPlaceholders, map);

    if (map.contains(name)) {
        return map[name]();
    }
    qCCritical(generic) << "ConcertRenamerData::value: Unknown tag:" << name;
    MediaElch_Debug_Unreachable();
    return "";
}

ELCH_NODISCARD bool ConcertRenamerData::passesCondition(const QString& name) const
{
    const QMap<QString, std::function<bool()>> map = {
        // clang-format off
        {"bluray", [this]() { return m_isBluRay; }},
        {"dvd",    [this]() { return m_isDvd; }},
        {"3D",     [this]() { return m_videoDetails.value(StreamDetails::VideoDetails::StereoMode) != ""; }},
        // clang-format on
    };

    MediaElch_Ensure_Condition_Matches_Placeholders(ConcertRenamerPlaceholders, map);

    return map.contains(name) //
               ? map[name]()
               : !value(name).isEmpty();
};

} // namespace mediaelch


ConcertRenamer::ConcertRenamer(RenamerConfig renamerConfig, RenamerDialog* dialog) : Renamer(renamerConfig, dialog)
{
}

ConcertRenamer::RenameError ConcertRenamer::renameConcert(Concert& concert)
{
    mediaelch::ConcertRenamerPlaceholders renamerPlaceholder;
    mediaelch::ConcertRenamerData renamerData{concert};

    QFileInfo concertInfo(concert.files().first().toString());
    QString fiCanonicalPath = concertInfo.canonicalPath();
    QDir dir(concertInfo.canonicalPath());
    QString newFolderName = m_config.directoryPattern;

    // " ", i.e. space, is the default for sanitizeFileName().
    QString delimiter = (m_config.replaceDelimiter) ? m_config.delimiter : " ";

    QString newFileName;
    QStringList newConcertFiles;
    QString parentDirName;

    MediaCenterInterface* mediaCenter = Manager::instance()->mediaCenterInterface();
    QString nfo = mediaCenter->nfoFilePath(&concert);

    bool errorOccured = false;

    for (const mediaelch::FilePath& file : concert.files()) {
        QFileInfo fi(file.toString());
        newConcertFiles.append(fi.fileName());
    }

    QDir chkDir(concertInfo.canonicalPath());
    chkDir.cdUp();

    const bool isBluRay = helper::isBluRay(chkDir.path());
    const bool isDvd = helper::isDvd(chkDir.path());

    renamerData.setIsBluRay(isBluRay);
    renamerData.setIsDvd(isDvd);

    if (isBluRay || isDvd) {
        parentDirName = dir.dirName();
        dir.cdUp();
    }

    if (!isBluRay && !isDvd && m_config.renameFiles) {
        newConcertFiles.clear();
        int partNo = 0;
        const auto videoDetails = concert.streamDetails()->videoDetails();
        for (const mediaelch::FilePath& file : concert.files()) {
            newFileName = (concert.files().count() == 1) ? m_config.filePattern : m_config.filePatternMulti;
            QFileInfo fi(file.toString());
            QString baseName = fi.completeBaseName();
            QDir currentDir = fi.dir();

            renamerData.setPartNo(++partNo);
            renamerData.setExtension(fi.suffix());
            newFileName = renamerPlaceholder.replace(newFileName, renamerData);

            // Sanitize + Replace Delimiter with the one chosen by the user
            helper::sanitizeFileName(newFileName, delimiter);

            if (fi.fileName() != newFileName) {
                if (!m_config.dryRun) {
                    const int row =
                        m_dialog->addResultToTable(fi.fileName(), newFileName, Renamer::RenameOperation::Rename);
                    if (!Renamer::rename(file.toString(), fi.canonicalPath() + "/" + newFileName)) {
                        m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
                        errorOccured = true;
                        continue;
                    }
                    newConcertFiles.append(newFileName);
                }

                QStringList filters;
                for (const QString& extra : m_extraFiles.fileGlob) {
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
                        }
                    }
                }
            } else {
                newConcertFiles.append(fi.fileName());
            }
        }

        const auto renameFileType = [&](QString filePath, DataFileType dataFileType) -> void {
            if (filePath.isEmpty()) {
                // File does not exist, e.g. there is no poster.
                return;
            }

            QVector<DataFile> files = Settings::instance()->dataFiles(dataFileType);
            if (files.isEmpty()) {
                return;
            }

            QString fileName = QFileInfo(filePath).fileName();
            QString newDataFileName =
                files.first().saveFileName(newFileName, SeasonNumber::NoSeason, concert.files().count() > 1);
            // Sanitize + Replace Delimiter with the one chosen by the user
            helper::sanitizeFileName(newDataFileName, delimiter);

            if (newDataFileName == fileName) {
                // File already has correct name
                return;
            }

            int row = m_dialog->addResultToTable(fileName, newDataFileName, Renamer::RenameOperation::Rename);
            if (m_config.dryRun) {
                return;
            }

            if (!Renamer::rename(filePath, fiCanonicalPath + "/" + newDataFileName)) {
                m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
            }
        };

        const auto renameImageType = [&](ImageType imageType) {
            DataFileType fileType = DataFile::dataFileTypeForImageType(imageType);
            renameFileType(mediaCenter->imageFileName(&concert, imageType), fileType);
        };

        renameFileType(nfo, DataFileType::ConcertNfo);

        renameImageType(ImageType::ConcertPoster);
        renameImageType(ImageType::ConcertBackdrop);
    }

    const auto videoDetails = concert.streamDetails()->videoDetails();
    int renameRow = -1;
    if (m_config.renameDirectories && concert.inSeparateFolder()) {
        renamerData.setPartNo(0);
        renamerData.setExtension("");
        newFileName = renamerPlaceholder.replace(newFolderName, renamerData);

        // Sanitize + Replace Delimiter with the one chosen by the user
        helper::sanitizeFolderName(newFolderName, delimiter);

        if (dir.dirName() != newFolderName) {
            renameRow = m_dialog->addResultToTable(dir.dirName(), newFolderName, Renamer::RenameOperation::Rename);
        }
    }

    QString newConcertFolder = dir.path();
    if (!m_config.dryRun && dir.dirName() != newFolderName && m_config.renameDirectories
        && concert.inSeparateFolder()) {
        QDir parentDir(dir.path());
        parentDir.cdUp();
        if (!Renamer::rename(dir, parentDir.path() + "/" + newFolderName)) {
            m_dialog->setResultStatus(renameRow, Renamer::RenameResult::Failed);
            errorOccured = true;
        } else {
            newConcertFolder = parentDir.path() + "/" + newFolderName;
        }
    }

    if (!errorOccured && !m_config.dryRun) {
        QStringList files;
        for (const QString& file : newConcertFiles) {
            QString f = newConcertFolder;
            if (isBluRay || isDvd) {
                f += "/" + parentDirName;
            }
            f += "/" + file;
            files << f;
        }
        concert.setFiles(files);
        mediaelch::ConcertPersistence persistence{*Manager::instance()->database()};
        persistence.update(&concert);
    }


    if (errorOccured) {
        return RenameError::Error;
    }
    return RenameError::None;
}
