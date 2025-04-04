#include "ui/renamer/TvShowRenamerDialog.h"

#include "database/TvShowPersistence.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "renamer/EpisodeRenamer.h"

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
        config.directoryPattern = ui->directoryNaming->text();
        renameTvShows(m_shows, config);
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
    if (!config.renameFiles || config.filePattern.isEmpty()) {
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
            m_renameErrorOccurred = true;
        }
    }
}

void TvShowRenamerDialog::renameTvShows(const QVector<TvShow*>& shows, const RenamerConfig& config)
{
    if (!config.renameDirectories || config.directoryPattern.isEmpty()) {
        return;
    }

    EpisodeRenamer renamer(config, this);

    for (TvShow* show : shows) {
        if (show->hasChanged()) {
            ui->results->append(
                tr("<b>TV Show</b> \"%1\" not renamed: It has been edited but is not saved").arg(show->title()));
            continue;
        }

        QApplication::processEvents();

        Renamer::RenameError err = renamer.renameTvShow(*show);
        if (err != Renamer::RenameError::None) {
            m_renameErrorOccurred = true;
        }
    }
}

void TvShowRenamerDialog::initPlaceholders()
{
    mediaelch::EpisodeRenamerPlaceholders placeholders;
    ui->placeholders->setPlaceholders(placeholders);
}

QStringList TvShowRenamerDialog::fileNameDefaults()
{
    return {
        "S<season>E<episode> - <title>.<extension>",
        "Season <season> Episode <episode> - <title>.<extension>",
    };
}

QStringList TvShowRenamerDialog::fileNameMultiDefaults()
{
    return {
        "S<season>E<episode> - <title>-part<partNo>.<extension>",
    };
}

QStringList TvShowRenamerDialog::directoryNameDefaults()
{
    return {
        "<title> (<year>)",
        "{movieset}<movieset> - {/movieset}<title> (<year>)",
        "<originalTitle> (<year>)",
        "<sortTitle>{imdbId} [<imdbId>]{/imdbId} (<year>)",
        "<sortTitle>{tmdbId} tmdbId-<tmdbId>{/tmdbId} (<year>)",
    };
}
