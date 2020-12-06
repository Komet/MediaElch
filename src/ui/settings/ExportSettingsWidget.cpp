#include "ui/settings/ExportSettingsWidget.h"
#include "ui_ExportSettingsWidget.h"

#include "export/ExportTemplate.h"
#include "export/ExportTemplateLoader.h"
#include "settings/DataFile.h"
#include "settings/Settings.h"
#include "ui/settings/ExportTemplateWidget.h"

ExportSettingsWidget::ExportSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ExportSettingsWidget)
{
    ui->setupUi(this);

    ui->exportTemplates->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // clang-format off
    connect(ExportTemplateLoader::instance(this), &ExportTemplateLoader::sigTemplateInstalled,   this, &ExportSettingsWidget::onTemplateInstalled);
    connect(ExportTemplateLoader::instance(this), &ExportTemplateLoader::sigTemplateUninstalled, this, &ExportSettingsWidget::onTemplateUninstalled);
    connect(ExportTemplateLoader::instance(this), &ExportTemplateLoader::sigTemplatesLoaded,     this, &ExportSettingsWidget::onTemplatesLoaded);
    // clang-format on
}

ExportSettingsWidget::~ExportSettingsWidget()
{
    delete ui;
}

void ExportSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
}


void ExportSettingsWidget::show()
{
    ui->themesErrorMessage->setText("");
    loadRemoteTemplates();
}

void ExportSettingsWidget::loadSettings()
{
}

void ExportSettingsWidget::saveSettings()
{
}

void ExportSettingsWidget::onTemplatesLoaded(QVector<ExportTemplate*> templates)
{
    ui->exportTemplates->clearContents();
    ui->exportTemplates->setRowCount(0);

    for (ExportTemplate* exportTemplate : templates) {
        auto* widget = new ExportTemplateWidget(ui->exportTemplates);
        widget->setExportTemplate(exportTemplate);

        const int row = ui->exportTemplates->rowCount();
        ui->exportTemplates->insertRow(row);
        ui->exportTemplates->setCellWidget(row, 0, widget);
        widget->adjustSize();
    }
}

void ExportSettingsWidget::onTemplateInstalled(ExportTemplate* exportTemplate, bool success)
{
    if (success) {
        ui->themesErrorMessage->setSuccessMessage(
            tr("Theme \"%1\" was successfully installed").arg(exportTemplate->name()));
    } else {
        ui->themesErrorMessage->setErrorMessage(
            tr("There was an error while processing the theme \"%1\"").arg(exportTemplate->name()));
    }
}

void ExportSettingsWidget::onTemplateUninstalled(ExportTemplate* exportTemplate, bool success)
{
    if (success) {
        ui->themesErrorMessage->setSuccessMessage(
            tr("Theme \"%1\" was successfully uninstalled").arg(exportTemplate->name()));
    } else {
        ui->themesErrorMessage->setErrorMessage(
            tr("There was an error while processing the theme \"%1\"").arg(exportTemplate->name()));
    }
}

void ExportSettingsWidget::loadRemoteTemplates()
{
    ExportTemplateLoader::instance()->getRemoteTemplates();
}
