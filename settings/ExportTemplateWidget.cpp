#include "ExportTemplateWidget.h"
#include "ui_ExportTemplateWidget.h"

#include "export/ExportTemplateLoader.h"

ExportTemplateWidget::ExportTemplateWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExportTemplateWidget)
{
    ui->setupUi(this);
}

ExportTemplateWidget::~ExportTemplateWidget()
{
    delete ui;
}

void ExportTemplateWidget::setExportTemplate(ExportTemplate *exportTemplate)
{
    m_exportTemplate = exportTemplate;
    ui->author->setText(tr("by %1").arg(exportTemplate->author()));
    ui->name->setText(exportTemplate->name());
    ui->description->setText(exportTemplate->description());
    ui->version->setText(tr("Version %1").arg(exportTemplate->version()));

    if (exportTemplate->updateAvailable()) {
        ui->btnInstall->setText(tr("Update"));
        ui->btnInstall->setButtonStyle(StyledPushButton::StyleYellow);
    } else if (exportTemplate->isInstalled()) {
        ui->btnInstall->setText(tr("Uninstall"));
        ui->btnInstall->setButtonStyle(StyledPushButton::StyleRed);
    } else if (exportTemplate->isRemote()) {
        ui->btnInstall->setText(tr("Install"));
        ui->btnInstall->setButtonStyle(StyledPushButton::StyleGreen);
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
