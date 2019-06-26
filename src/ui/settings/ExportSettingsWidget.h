#pragma once

#include "globals/Globals.h"

#include <QWidget>

namespace Ui {
class ExportSettingsWidget;
}

class Settings;
class ExportTemplate;

class ExportSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExportSettingsWidget(QWidget* parent = nullptr);
    ~ExportSettingsWidget() override;

    void setSettings(Settings& settings);
    void show();
    void loadSettings();
    void saveSettings();

private slots:
    void onTemplatesLoaded(QVector<ExportTemplate*> templates);
    void onTemplateInstalled(ExportTemplate* exportTemplate, bool success);
    void onTemplateUninstalled(ExportTemplate* exportTemplate, bool success);

private:
    Ui::ExportSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;

    void loadRemoteTemplates();
};
