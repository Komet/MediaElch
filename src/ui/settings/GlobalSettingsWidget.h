#pragma once

#include "globals/MediaDirectory.h"

#include <QComboBox>
#include <QWidget>

namespace Ui {
class GlobalSettingsWidget;
}

class Settings;

class GlobalSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlobalSettingsWidget(QWidget* parent = nullptr);
    ~GlobalSettingsWidget() override;

    void setSettings(Settings& settings);

    void loadSettings();
    void saveSettings();

private slots:
    void chooseDirToAdd();
    void addDir(mediaelch::MediaDirectory directory,
        mediaelch::MediaDirectoryType dirType = mediaelch::MediaDirectoryType::Movies);
    void removeDir();
    void organize();
    void dirListRowChanged(int currentRow);
    void dirListEntryChanged(int row, int column);
    void onDirTypeChanged(QComboBox* box);

private:
    Ui::GlobalSettingsWidget* ui = nullptr;
    Settings* m_settings = nullptr;
};
