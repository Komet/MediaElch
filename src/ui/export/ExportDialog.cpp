#include "ExportDialog.h"
#include "ui_ExportDialog.h"

#include "export/ExportTemplateLoader.h"
#include "export/MediaExport.h"
#include "globals/Manager.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <string>

ExportDialog::ExportDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog),
    m_exporter{new mediaelch::MediaExport}
{
    ui->setupUi(this);
#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    connect(m_exporter, &mediaelch::MediaExport::sigItemExported, this, [&](int itemsExported) {
        ui->progressBar->setValue(itemsExported);
    });
}

ExportDialog::~ExportDialog()
{
    delete ui;
    delete m_exporter;
}

int ExportDialog::exec()
{
    resetProgress();

    QVector<ExportTemplate*> templates = ExportTemplateLoader::instance()->installedTemplates();

    ui->message->clear();
    ui->btnExport->setEnabled(!templates.isEmpty());

    if (templates.isEmpty()) {
        ui->message->setErrorMessage(tr("You need to install at least one theme."));
    }

    ui->comboTheme->clear();
    for (const ExportTemplate* exportTemplate : templates) {
        ui->comboTheme->addItem(exportTemplate->name(), exportTemplate->identifier());
    }

    // current theme: first entry
    onThemeChanged();
    adjustSize();

    return QDialog::exec();
}

void ExportDialog::onBtnExport()
{
    ui->message->clear();

    int index = ui->comboTheme->currentIndex();
    if (index < 0 || index >= ui->comboTheme->count()) {
        return;
    }

    QVector<ExportTemplate::ExportSection> sections;
    if (ui->chkConcerts->isEnabled() && ui->chkConcerts->isChecked()) {
        sections << ExportTemplate::ExportSection::Concerts;
    }
    if (ui->chkMovies->isEnabled() && ui->chkMovies->isChecked()) {
        sections << ExportTemplate::ExportSection::Movies;
    }
    if (ui->chkTvShows->isEnabled() && ui->chkTvShows->isChecked()) {
        sections << ExportTemplate::ExportSection::TvShows;
    }

    if (sections.isEmpty()) {
        ui->message->setErrorMessage(tr("You need to select at least one media type to export."));
        return;
    }

    ExportTemplate* exportTemplate =
        ExportTemplateLoader::instance()->getTemplateByIdentifier(ui->comboTheme->itemData(index).toString());
    if (exportTemplate == nullptr) {
        return;
    }

    resetProgress();

    QString location = QFileDialog::getExistingDirectory(this, tr("Export directory"), QDir::homePath());
    if (location.isEmpty()) {
        ui->message->setErrorMessage(tr("Export aborted. Not directory was selected."));
        return;
    }

    QDir dir(location);
    QString subDir =
        QStringLiteral("MediaElch Export %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm"));
    if (!dir.mkdir(subDir)) {
        ui->message->setErrorMessage(tr("Could not create export directory."));
        return;
    }
    QDir::setCurrent(location + "/" + subDir);

    ui->btnExport->setEnabled(false);

    int itemsToExport = 0;
    if (sections.contains(ExportTemplate::ExportSection::Concerts)) {
        itemsToExport += Manager::instance()->concertModel()->concerts().count();
    }
    if (sections.contains(ExportTemplate::ExportSection::Movies)) {
        itemsToExport += Manager::instance()->movieModel()->movies().count();
    }
    if (sections.contains(ExportTemplate::ExportSection::TvShows)) {
        for (TvShow* show : Manager::instance()->tvShowModel()->tvShows()) {
            ++itemsToExport;
            itemsToExport += show->episodeCount();
        }
    }

    ui->progressBar->setRange(0, itemsToExport);

    // Create the base structure
    exportTemplate->copyTo(QDir::currentPath());

    // Export movies
    if (sections.contains(ExportTemplate::ExportSection::Movies)) {
        if (m_canceled) {
            return;
        }
        m_exporter->parseAndSaveMovies(
            QDir::currentPath(), exportTemplate, Manager::instance()->movieModel()->movies());
    }

    // Export TV Shows
    if (sections.contains(ExportTemplate::ExportSection::TvShows)) {
        if (m_canceled) {
            return;
        }
        m_exporter->parseAndSaveTvShows(
            QDir::currentPath(), exportTemplate, Manager::instance()->tvShowModel()->tvShows());
    }

    // Export Concerts
    if (sections.contains(ExportTemplate::ExportSection::Concerts)) {
        if (m_canceled) {
            return;
        }
        m_exporter->parseAndSaveConcerts(
            QDir::currentPath(), exportTemplate, Manager::instance()->concertModel()->concerts());
    }

    ui->progressBar->setValue(ui->progressBar->maximum());
    ui->message->setSuccessMessage(tr("Export completed."));
    ui->btnExport->setEnabled(true);
}

void ExportDialog::onBtnClose()
{
    m_exporter->cancel();
    m_canceled = true;
    QDialog::reject();
}

void ExportDialog::onThemeChanged()
{
    int index = ui->comboTheme->currentIndex();
    if (index < 0 || index >= ui->comboTheme->count()) {
        return;
    }

    ExportTemplate* exportTemplate =
        ExportTemplateLoader::instance()->getTemplateByIdentifier(ui->comboTheme->itemData(index).toString());
    if (exportTemplate == nullptr) {
        return;
    }
    resetProgress();

    ui->chkConcerts->setEnabled(exportTemplate->exportSections().contains(ExportTemplate::ExportSection::Concerts));
    ui->chkMovies->setEnabled(exportTemplate->exportSections().contains(ExportTemplate::ExportSection::Movies));
    ui->chkTvShows->setEnabled(exportTemplate->exportSections().contains(ExportTemplate::ExportSection::TvShows));
}

void ExportDialog::resetProgress()
{
    ui->message->clear();
    m_canceled = false;
    m_exporter->reset();
    ui->progressBar->setValue(0);
}
