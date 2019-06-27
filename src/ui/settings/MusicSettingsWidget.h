#pragma once

#include "globals/Globals.h"

#include <QWidget>

namespace Ui {
class MusicSettingsWidget;
}

class Settings;

class MusicSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MusicSettingsWidget(QWidget* parent = nullptr);
    ~MusicSettingsWidget() override;

    void setSettings(Settings& settings);
    void loadSettings();
    void saveSettings();

private:
    Ui::MusicSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;
};
