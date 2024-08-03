#pragma once

#include "scrapers/ScraperInfos.h"

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

signals:
    void saveSettings();

private slots:
    void onSaveSettings();

private:
    Ui::ScraperSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;

    QComboBox* comboForMovieScraperInfo(MovieScraperInfo info);
    QString titleForMovieScraperInfo(MovieScraperInfo info);
};
