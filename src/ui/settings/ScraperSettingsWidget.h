#pragma once

#include "globals/ScraperInfos.h"

#include <QComboBox>
#include <QWidget>

namespace Ui {
class ScraperSettingsWidget;
}

class Settings;

namespace mediaelch {
namespace scraper {
class MovieScraper;
}
} // namespace mediaelch

class ScraperSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScraperSettingsWidget(QWidget* parent = nullptr);
    ~ScraperSettingsWidget() override;

    void setSettings(Settings& settings);
    void loadSettings();
    void saveSettings();

private slots:
    void onShowAdultScrapers();

private:
    Ui::ScraperSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;
    QMap<const mediaelch::scraper::MovieScraper*, int> m_scraperRows;

    QComboBox* comboForMovieScraperInfo(MovieScraperInfo info);
    QString titleForMovieScraperInfo(MovieScraperInfo info);
};
