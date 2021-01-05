#pragma once

#include "data/Locale.h"
#include "globals/ScraperInfos.h"

#include <QListWidgetItem>
#include <QString>
#include <QWidget>

namespace Ui {
class TvScraperSettingsWidget;
}

namespace mediaelch {
namespace scraper {
class TvScraper;
}
} // namespace mediaelch

class Settings;
class ScraperSettings;

class TvScraperSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TvScraperSettingsWidget(QWidget* parent = nullptr);
    ~TvScraperSettingsWidget() override;

    void setSettings(Settings& settings);
    void loadSettings();
    void saveSettings();

private slots:
    void scraperChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onLanguageChanged();

private:
    ScraperSettings* currentSettings();
    void setupScraperDetails();
    void setupLanguageBox();

private:
    Ui::TvScraperSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;
    mediaelch::scraper::TvScraper* m_currentScraper = nullptr;
};
