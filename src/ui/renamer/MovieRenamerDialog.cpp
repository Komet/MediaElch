#include "ui/renamer/MovieRenamerDialog.h"

#include "globals/Helper.h"
#include "globals/Manager.h"
#include "renamer/MovieRenamer.h"

#include "ui_RenamerDialog.h"


MovieRenamerDialog::MovieRenamerDialog(QWidget* parent) : RenamerDialog(parent)
{
    m_renameType = RenameType::Movies;
}

MovieRenamerDialog::~MovieRenamerDialog() = default;

void MovieRenamerDialog::setMovies(QVector<Movie*> movies)
{
    m_movies = movies;
}

void MovieRenamerDialog::renameType(const bool isDryRun)
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

    config.directoryPattern = ui->directoryNaming->text();
    config.renameDirectories = ui->chkDirectoryNaming->isChecked();
    renameMovies(m_movies, config);

    if (isDryRun) {
        m_filesRenamed = true;
    }
    ui->results->append("<span style=\"color:#01a800;\"><b>" + tr("Finished") + "</b></span>");
}

void MovieRenamerDialog::rejectImpl()
{
    m_movies.clear();
}

QString MovieRenamerDialog::dialogInfoLabel()
{
    return tr("%n movies will be renamed", "", qsizetype_to_int(m_movies.count()));
}


void MovieRenamerDialog::renameMovies(QVector<Movie*> movies, const RenamerConfig& config)
{
    if ((config.renameFiles && config.filePattern.isEmpty())
        || (config.renameDirectories && config.directoryPattern.isEmpty())) {
        return;
    }

    MovieRenamer renamer(config, this);
    for (Movie* movie : movies) {
        if (movie->files().isEmpty() || (movie->files().count() > 1 && config.filePatternMulti.isEmpty())) {
            continue;
        }
        if (movie->hasChanged()) {
            ui->results->append(
                tr("<b>Movie</b> \"%1\" not renamed: It has been edited but is not saved").arg(movie->name()));
            continue;
        }

        QApplication::processEvents();

        Renamer::RenameError err = renamer.renameMovie(*movie);
        if (err != Renamer::RenameError::None) {
            m_renameErrorOccured = true;
        }
    }
}

QStringList MovieRenamerDialog::fileNameDefaults()
{
    return {
        "<title>.<extension>",
        "<originalTitle>.<extension>",
        "<title>{tmdbId} tmdbId-<tmdbId>{/tmdbId}{imdbId} imdbId-<imdbId>{/imdbId} (<year>).<extension>",
    };
}

QStringList MovieRenamerDialog::fileNameMultiDefaults()
{
    return {
        "<title>-part<partNo>.<extension>",
        "<originalTitle>-part<partNo>.<extension>",
    };
}

QStringList MovieRenamerDialog::directoryNameDefaults()
{
    return {
        "<title> (<year>)",
        "{movieset}<movieset> - {/movieset}<title> (<year>)",
        "<originalTitle> (<year>)",
        "<sortTitle>{imdbId} [<imdbId>]{/imdbId} (<year>)",
        "<sortTitle>{tmdbId} tmdbId-<tmdbId>{/tmdbId} (<year>)",
    };
}
