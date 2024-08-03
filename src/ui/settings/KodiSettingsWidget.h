#pragma once

#include "globals/Globals.h"
#include "media_center/KodiVersion.h"
#include "settings/KodiSettings.h"

#include <QWidget>

namespace Ui {
class KodiSettingsWidget;
}

class Settings;

class KodiSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KodiSettingsWidget(QWidget* parent = nullptr);
    ~KodiSettingsWidget() override;

    void init(mediaelch::KodiSettings* settings);

private:
    void setKodiVersion(mediaelch::KodiVersion kodiVersion);

private:
    Ui::KodiSettingsWidget* ui{nullptr};
};
