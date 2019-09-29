#include "ExportDialog.h"
#include "ui_ExportDialog.h"

#include "export/ExportTemplateLoader.h"
#include "export/MediaExport.h"
#include "globals/Manager.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <string>

ExportDialog::ExportDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

int ExportDialog::exec()
{
    resetProgress();

    QVector<ExportTemplate*> templates = ExportTemplateLoader::instance()->installedTemplates();
    ui->btnExport->setEnabled(!templates.isEmpty());

    if (templates.isEmpty()) {
        ui->message->setErrorMessage(tr("You need to install at least one theme."));
    }

    // fill combo box with templates
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
    resetProgress();

    auto sections = sectionsToExport();
    if (sections.isEmpty()) {
        ui->message->setErrorMessage(tr("You need to select at least one media type to export."));
        return;
    }

    ExportTemplate* exportTemplate = ExportTemplateLoader::instance()->getTemplateByIdentifier(themeName());
    if (exportTemplate == nullptr) {
        warnAboutInvalidTheme();
        return;
    }

    QString location = QFileDialog::getExistingDirectory(this, tr("Export directory"), QDir::homePath());
    if (location.isEmpty()) {
        ui->message->setErrorMessage(tr("Export aborted. No directory was selected."));
        return;
    }

    QDir dir(location);
    QString subDir =
        QStringLiteral("MediaElch Export %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm"));
    if (!dir.mkdir(subDir)) {
        ui->message->setErrorMessage(tr("Could not create export directory."));
        return;
    }

    mediaelch::MediaExport exporter(m_canceled);
    connect(&exporter, &mediaelch::MediaExport::sigItemExported, [&]() { //
        ui->progressBar->setValue(++m_itemsExported);
    });

    ui->btnExport->setEnabled(false);
    ui->progressBar->setRange(0, libraryItemCount(sections));

    exporter.doExport(*exportTemplate, location + "/" + subDir, sections);

    ui->progressBar->setValue(ui->progressBar->maximum());

    if (m_canceled) {
        ui->message->setErrorMessage(tr("Export canceled."));
        qInfo() << "[Export] Cancelled";
    } else {
        ui->message->setSuccessMessage(tr("Export completed."));
        qInfo() << "[Export] Finished successfully";
    }
    ui->btnExport->setEnabled(true);
}

void ExportDialog::onBtnClose()
{
    m_canceled = true;
    QDialog::reject();
}

void ExportDialog::onThemeChanged()
{
    resetProgress();

    ExportTemplate* exportTemplate = ExportTemplateLoader::instance()->getTemplateByIdentifier(themeName());
    if (exportTemplate == nullptr) {
        warnAboutInvalidTheme();
        return;
    }

    const auto sections = exportTemplate->exportSections();
    auto initBox = [&sections](QCheckBox* box, ExportTemplate::ExportSection type) {
        const bool isAvailable = sections.contains(type);
        box->setEnabled(isAvailable);
        if (!isAvailable) {
            box->setChecked(false);
        }
    };

    initBox(ui->chkConcerts, ExportTemplate::ExportSection::Concerts);
    initBox(ui->chkMovies, ExportTemplate::ExportSection::Movies);
    initBox(ui->chkTvShows, ExportTemplate::ExportSection::TvShows);
}

void ExportDialog::resetProgress()
{
    m_itemsExported = 0;
    ui->message->clear();
    m_canceled = false;
    ui->progressBar->setValue(0);
}

void ExportDialog::warnAboutInvalidTheme()
{
    qCritical() << "[ExportDialog] Internal Error: Couldn't find selected theme:" << themeName();
    ui->message->setErrorMessage(tr("Selected theme not found! Try to restart MediaElch."));
}

QString ExportDialog::themeName() const
{
    int index = ui->comboTheme->currentIndex();
    if (index < 0 || index >= ui->comboTheme->count()) {
        return "";
    }
    return ui->comboTheme->itemData(index).toString();
}

QVector<ExportTemplate::ExportSection> ExportDialog::sectionsToExport() const
{
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
    return sections;
}

int ExportDialog::libraryItemCount(const QVector<ExportTemplate::ExportSection>& sections) const
{
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
    return itemsToExport;
}
