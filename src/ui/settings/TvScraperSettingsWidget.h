#pragma once

#include "globals/ScraperInfos.h"

#include <QComboBox>
#include <QWidget>

namespace Ui {
class TvScraperSettingsWidget;
}

class Settings;

class TvScraperSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TvScraperSettingsWidget(QWidget* parent = nullptr);
    ~TvScraperSettingsWidget() override;

    void setSettings(Settings& settings);
    void loadSettings();
    void saveSettings();

private:
    Ui::TvScraperSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;

    QComboBox* comboForTvScraperInfo(ShowScraperInfos info);
    QString titleForTvScraperInfo(ShowScraperInfos info);
};
