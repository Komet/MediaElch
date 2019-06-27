#pragma once

#include "globals/Globals.h"

#include <QWidget>

namespace Ui {
class KodiSettingsWidget;
}

class Settings;

class KodiSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KodiSettingsWidget(QWidget* parent = nullptr);
    ~KodiSettingsWidget() override;

    void setSettings(Settings& settings);
    void loadSettings();
    void saveSettings();

private:
    Ui::KodiSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;
};
