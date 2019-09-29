#pragma once

#include "globals/Globals.h"

#include <QWidget>

namespace Ui {
class ImportSettingsWidget;
}

class Settings;

class ImportSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImportSettingsWidget(QWidget* parent = nullptr);
    ~ImportSettingsWidget() override;

    void setSettings(Settings& settings);

    void loadSettings();
    void saveSettings();

private slots:
    void onChooseUnrar();
    void onChooseMakeMkvCon();

private:
    Ui::ImportSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;
};
