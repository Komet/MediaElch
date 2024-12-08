#include "ui/renamer/TvShowRenamerDialog.h"

#include "globals/Helper.h"
#include "globals/Manager.h"
#include "renamer/EpisodeRenamer.h"
#include "database/TvShowPersistence.h"

#include "ui_RenamerDialog.h"

#include <QDir>
#include <QFile>

TvShowRenamerDialog::TvShowRenamerDialog(QWidget* parent) : RenamerDialog(parent)
{
    m_renameType = RenameType::TvShows;
}

TvShowRenamerDialog::~TvShowRenamerDialog() = default;

void TvShowRenamerDialog::setShows(QVector<TvShow*> shows)
{
    m_shows = shows;
}

void TvShowRenamerDialog::setEpisodes(QVector<TvShowEpisode*> episodes)
{
    m_episodes = episodes;
}

void TvShowRenamerDialog::renameType(const bool isDryRun)
{
    ui->tabWidget->setCurrentIndex(1);
    ui->results->clear();
    ui->resultsTable->setRowCount(0);

    RenamerConfig config;
    config.dryRun = isDryRun;
    config.filePattern = ui->fileNaming->text();
    config.filePatternMulti = ui->fileNamingMulti->text();
    config.renameFiles = ui->chkFileNaming->isChecked();

    config.replaceDelimiter = ui->chkReplaceDelimiter->isChecked();
    config.delimiter = ui->newDelimiterNaming->currentText();

    config.directoryPattern = ui->seasonNaming->text();
    config.renameDirectories = ui->chkSeasonDirectories->isChecked();
    renameEpisodes(m_episodes, config);
    if (config.renameDirectories) {
        renameTvShows(m_shows, ui->directoryNaming->text(), isDryRun);
    }

    if (isDryRun) {
        m_filesRenamed = true;
    }
    ui->results->append("<span style=\"color:#01a800;\"><b>" + tr("Finished") + "</b></span>");
}

void TvShowRenamerDialog::rejectImpl()
{
    m_shows.clear();
    m_episodes.clear();
}

QString TvShowRenamerDialog::dialogInfoLabel()
{
    return tr("%n TV shows and %1", "", qsizetype_to_int(m_shows.count()))
        .arg(tr("%n episodes will be renamed", "", qsizetype_to_int(m_episodes.count())));
}

void TvShowRenamerDialog::renameEpisodes(QVector<TvShowEpisode*> episodes, const RenamerConfig& config)
{
    if (config.renameFiles && config.filePattern.isEmpty()) {
        return;
    }

    EpisodeRenamer renamer(config, this);
    QVector<TvShowEpisode*> episodesRenamed;

    for (TvShowEpisode* episode : episodes) {
        if (episode->files().isEmpty() || (episode->files().count() > 1 && config.filePatternMulti.isEmpty())
            || episodesRenamed.contains(episode)) {
            continue;
        }
        if (episode->hasChanged()) {
            ui->results->append(
                tr("<b>Episode</b> \"%1\" not renamed: It has been edited but is not saved").arg(episode->title()));
            continue;
        }

        QApplication::processEvents();

        Renamer::RenameError err = renamer.renameEpisode(*episode, episodesRenamed);
        if (err != Renamer::RenameError::None) {
            m_renameErrorOccured = true;
        }
    }
}

void TvShowRenamerDialog::renameTvShows(const QVector<TvShow*>& shows,
    const QString& directoryPattern,
    const bool& dryRun)
{
    if (directoryPattern.isEmpty()) {
        return;
    }

    mediaelch::TvShowPersistence persistence{*Manager::instance()->database()};
    for (TvShow* show : shows) {
        if (show->hasChanged()) {
            ui->results->append(
                tr("<b>TV Show</b> \"%1\" not renamed: It has been edited but is not saved").arg(show->title()));
            continue;
        }

        QDir dir(show->dir().toString());
        QString newFolderName = directoryPattern;
        Renamer::replace(newFolderName, "title", show->title());
        Renamer::replace(newFolderName, "showTitle", show->title());
        Renamer::replaceCondition(newFolderName, "tmdbId", show->tmdbId().toString());
        Renamer::replace(newFolderName, "year", show->firstAired().toString("yyyy"));
        helper::sanitizeFolderName(newFolderName);
        if (newFolderName != dir.dirName()) {
            const int row = addResultToTable(dir.dirName(), newFolderName, Renamer::RenameOperation::Rename);
            QDir parentDir(dir.path());
            parentDir.cdUp();
            if (dryRun) {
                continue;
            }
            if (!Renamer::rename(dir, parentDir.absolutePath() + "/" + newFolderName)) {
                setResultStatus(row, Renamer::RenameResult::Failed);
                m_renameErrorOccured = true;
                continue;
            }
            const QString newShowDir = parentDir.absolutePath() + "/" + newFolderName;
            const QString oldShowDir = show->dir().toString();
            show->setDir(mediaelch::DirectoryPath(newShowDir));
            persistence.update(show);
            for (TvShowEpisode* episode : show->episodes()) {
                QStringList files;
                for (const mediaelch::FilePath& file : episode->files()) {
                    files << newShowDir + file.toString().mid(oldShowDir.length());
                }
                episode->setFiles(files);
                persistence.update(episode);
            }
        }
    }
}
