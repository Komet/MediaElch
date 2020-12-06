#pragma once

#include "globals/ScraperInfos.h"

#include <QComboBox>
#include <QWidget>

namespace Ui {
class CustomTvScraperSettingsWidget;
}

namespace mediaelch {
namespace scraper {
class CustomTvScraper;
}
} // namespace mediaelch

class Settings;

class CustomTvScraperSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomTvScraperSettingsWidget(QWidget* parent = nullptr);
    ~CustomTvScraperSettingsWidget() override;

    void setSettings(Settings& settings);
    void loadSettings();
    void saveSettings();

private:
    Ui::CustomTvScraperSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;

    QComboBox* comboForTvScraperInfo(ShowScraperInfo info);
    QComboBox* comboForEpisodeInfo(EpisodeScraperInfo info);
};
