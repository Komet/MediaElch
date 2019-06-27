#pragma once

#include "globals/Globals.h"

#include <QWidget>

namespace Ui {
class MovieSettingsWidget;
}

class Settings;

class MovieSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovieSettingsWidget(QWidget* parent = nullptr);
    ~MovieSettingsWidget() override;

    void setSettings(Settings& settings);
    void loadSettings();
    void saveSettings();

private slots:
    void onComboMovieSetArtworkChanged();
    void onChooseMovieSetArtworkDir();

private:
    Ui::MovieSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;
};
