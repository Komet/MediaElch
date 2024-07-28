#pragma once

#include "settings/Settings.h"

#include <QMainWindow>

namespace Ui {
class SettingsDebugTable;
}

class SettingsDebugTable : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsDebugTable(Settings& settings, QWidget* parent = nullptr);
    ~SettingsDebugTable() override;

private:
    void setupTable();

private:
    Ui::SettingsDebugTable* ui;
    Settings& m_settings;
};
