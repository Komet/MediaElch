#pragma once

#include "globals/Globals.h"

#include <QWidget>

namespace Ui {
class TvShowSettingsWidget;
}

class Settings;

class TvShowSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowSettingsWidget(QWidget* parent = nullptr);
    ~TvShowSettingsWidget() override;

    void setSettings(Settings& settings);
    void loadSettings();
    void saveSettings();

private:
    Ui::TvShowSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;
};
