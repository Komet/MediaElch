#pragma once

#include "globals/ScraperInfos.h"

#include <QComboBox>
#include <QWidget>

namespace Ui {
class ScraperSettingsWidget;
}

class Settings;
class MovieScraperInterface;

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
    QMap<const MovieScraperInterface*, int> m_scraperRows;

    QComboBox* comboForMovieScraperInfo(MovieScraperInfos info);
    QString titleForMovieScraperInfo(MovieScraperInfos info);
    QComboBox* comboForTvScraperInfo(TvShowScraperInfos info);
    QString titleForTvScraperInfo(TvShowScraperInfos info);
};
