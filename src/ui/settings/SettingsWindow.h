#pragma once

#include "globals/Globals.h"
#include "settings/Settings.h"

#include <QCloseEvent>
#include <QListWidgetItem>
#include <QMainWindow>

namespace Ui {
class SettingsWindow;
}

class Settings;

class SettingsWindowConfiguration
{
public:
    explicit SettingsWindowConfiguration(Settings& settings);

    void init();

    ELCH_NODISCARD QSize settingsWindowSize();
    void setSettingsWindowSize(QSize settingsWindowSize);

    ELCH_NODISCARD QPoint settingsWindowPosition();
    void setSettingsWindowPosition(QPoint settingsWindowPosition);

private:
    Settings& m_settings;
};

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsWindow(Settings& settings, QWidget* parent = nullptr);
    ~SettingsWindow() override;

public slots:
    void show();

signals:
    void sigSaved();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onAction();
    void onSave();
    void onCancel();

private:
    Ui::SettingsWindow* ui{nullptr};
    Settings& m_settings;
    SettingsWindowConfiguration m_config;
    QColor m_buttonColor;
    QColor m_buttonActiveColor;
    bool m_saveCloseHandled{false};

    void loadSettings();
    void saveSettings();
};
