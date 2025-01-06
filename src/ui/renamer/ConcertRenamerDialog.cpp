#include "ui/renamer/ConcertRenamerDialog.h"

#include "globals/Helper.h"
#include "globals/Manager.h"
#include "renamer/ConcertRenamer.h"

#include "ui_RenamerDialog.h"

ConcertRenamerDialog::ConcertRenamerDialog(QWidget* parent) : RenamerDialog(parent)
{
    m_renameType = RenameType::Concerts;
}

ConcertRenamerDialog::~ConcertRenamerDialog() = default;

void ConcertRenamerDialog::setConcerts(QVector<Concert*> concerts)
{
    m_concerts = concerts;
}

void ConcertRenamerDialog::renameType(const bool isDryRun)
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
    renameConcerts(m_concerts, config);

    if (isDryRun) {
        m_filesRenamed = true;
    }
    ui->results->append("<span style=\"color:#01a800;\"><b>" + tr("Finished") + "</b></span>");
}

void ConcertRenamerDialog::rejectImpl()
{
    m_concerts.clear();
}

QString ConcertRenamerDialog::dialogInfoLabel()
{
    return tr("%n concerts will be renamed", "", qsizetype_to_int(m_concerts.count()));
}

void ConcertRenamerDialog::renameConcerts(QVector<Concert*> concerts, const RenamerConfig& config)
{
    if ((config.renameFiles && config.filePattern.isEmpty())
        || (config.renameDirectories && config.directoryPattern.isEmpty())) {
        return;
    }

    ConcertRenamer renamer(config, this);

    for (Concert* concert : concerts) {
        if (concert->files().isEmpty() || (concert->files().count() > 1 && config.filePatternMulti.isEmpty())) {
            continue;
        }
        if (concert->hasChanged()) {
            ui->results->append(
                tr("<b>Concert</b> \"%1\" not renamed: It has been edited but is not saved").arg(concert->title()));
            continue;
        }

        QApplication::processEvents();

        Renamer::RenameError err = renamer.renameConcert(*concert);
        if (err != Renamer::RenameError::None) {
            m_renameErrorOccurred = true;
        }
    }
}

QStringList ConcertRenamerDialog::fileNameDefaults()
{
    return {
        "<title>.<extension>",
        "<originalTitle>.<extension>",
    };
}

QStringList ConcertRenamerDialog::fileNameMultiDefaults()
{
    return {
        "<title>-part<partNo>.<extension>",
        "<originalTitle>-part<partNo>.<extension>",
    };
}

QStringList ConcertRenamerDialog::directoryNameDefaults()
{
    return {
        "<title> (<year>)",
        "{movieset}<movieset> - {/movieset}<title> (<year>)",
        "<originalTitle> (<year>)",
        "<sortTitle>{imdbId} [<imdbId>]{/imdbId} (<year>)",
    };
}
