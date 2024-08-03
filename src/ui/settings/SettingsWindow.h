#pragma once

#include "globals/Globals.h"

#include <QCloseEvent>
#include <QListWidgetItem>
#include <QMainWindow>

namespace Ui {
class SettingsWindow;
}

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
    void onSave();
    void onCancel();

private:
    Ui::SettingsWindow* ui{nullptr};
    Settings* m_settings{nullptr};
    QColor m_buttonColor;
    QColor m_buttonActiveColor;
    bool m_saveCloseHandled{false};

    void loadSettings();
    void saveSettings();
};
