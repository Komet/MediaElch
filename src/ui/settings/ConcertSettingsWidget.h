#pragma once

#include "globals/Globals.h"

#include <QWidget>

namespace Ui {
class ConcertSettingsWidget;
}

class Settings;

class ConcertSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConcertSettingsWidget(QWidget* parent = nullptr);
    ~ConcertSettingsWidget() override;

    void setSettings(Settings& settings);
    void loadSettings();
    void saveSettings();

private:
    Ui::ConcertSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;
};
