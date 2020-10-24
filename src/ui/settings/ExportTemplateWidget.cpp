#include "ExportTemplateWidget.h"
#include "ui_ExportTemplateWidget.h"

#include "export/ExportTemplateLoader.h"
#include "globals/Helper.h"

ExportTemplateWidget::ExportTemplateWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ExportTemplateWidget)
{
    ui->setupUi(this);
#ifdef Q_OS_MAC
    QFont smallFont = ui->description->font();
    smallFont.setPointSize(smallFont.pointSize() - 1);
    ui->description->setFont(smallFont);
#endif
}

ExportTemplateWidget::~ExportTemplateWidget()
{
    delete ui;
}

void ExportTemplateWidget::setExportTemplate(ExportTemplate* exportTemplate)
{
    m_exportTemplate = exportTemplate;
    ui->author->setText(tr("by %1").arg(exportTemplate->author()));
    ui->name->setText(exportTemplate->name());
    if (!exportTemplate->website().isEmpty()) {
        ui->website->setText(exportTemplate->website());
    } else {
        ui->website->setText(tr("No website available."));
    }
    ui->description->setText(exportTemplate->description());
    ui->version->setText(tr("Version %1").arg(exportTemplate->version()));

    if (exportTemplate->updateAvailable()) {
        ui->btnInstall->setText(tr("Update"));
        helper::setButtonStyle(ui->btnInstall, helper::ButtonWarning);
    } else if (exportTemplate->isInstalled()) {
        ui->btnInstall->setText(tr("Uninstall"));
        helper::setButtonStyle(ui->btnInstall, helper::ButtonDanger);
    } else if (exportTemplate->isRemote()) {
        ui->btnInstall->setText(tr("Install"));
        helper::setButtonStyle(ui->btnInstall, helper::ButtonSuccess);
    }
}

void ExportTemplateWidget::onBtnInstall()
{
    if (m_exportTemplate->updateAvailable()) {
        ExportTemplateLoader::instance()->installTemplate(m_exportTemplate);
        ui->btnInstall->setEnabled(false);
        ui->btnInstall->setText(tr("Updating..."));
    } else if (!m_exportTemplate->isInstalled() && m_exportTemplate->isRemote()) {
        ExportTemplateLoader::instance()->installTemplate(m_exportTemplate);
        ui->btnInstall->setEnabled(false);
        ui->btnInstall->setText(tr("Installing..."));
    } else if (m_exportTemplate->isInstalled()) {
        ExportTemplateLoader::instance()->uninstallTemplate(m_exportTemplate);
        ui->btnInstall->setEnabled(false);
        ui->btnInstall->setText(tr("Uninstalling..."));
    }
}
