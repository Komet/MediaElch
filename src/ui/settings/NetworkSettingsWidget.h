#pragma once

#include "globals/Globals.h"

#include <QWidget>

namespace Ui {
class NetworkSettingsWidget;
}

class Settings;

class NetworkSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkSettingsWidget(QWidget* parent = nullptr);
    ~NetworkSettingsWidget() override;

    void setSettings(Settings& settings);
    void loadSettings();
    void saveSettings();

private slots:
    void onUseProxy();

private:
    Ui::NetworkSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;
};
