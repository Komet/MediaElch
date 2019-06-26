#pragma once

#include "globals/Globals.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QListWidgetItem>
#include <QMainWindow>

namespace Ui {
class SettingsWindow;
}

class MovieScraperInterface;
class Settings;

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget* parent = nullptr);
    ~SettingsWindow() override;

public slots:
    void show();

signals:
    void sigSaved();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onAction();
    void onComboMovieSetArtworkChanged();
    void onChooseMovieSetArtworkDir();
    void onUseProxy();
    void onSave();
    void onCancel();
    void onShowAdultScrapers();

private:
    Ui::SettingsWindow* ui = nullptr;
    Settings* m_settings = nullptr;
    QMap<const MovieScraperInterface*, int> m_scraperRows;
    QColor m_buttonColor;
    QColor m_buttonActiveColor;

    void loadSettings();
    void saveSettings();
    QComboBox* comboForMovieScraperInfo(MovieScraperInfos info);
    QString titleForMovieScraperInfo(MovieScraperInfos info);
    QComboBox* comboForTvScraperInfo(TvShowScraperInfos info);
    QString titleForTvScraperInfo(TvShowScraperInfos info);
};
