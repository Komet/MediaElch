#pragma once

#include "scrapers/tv_show/tvmaze/TvMazeConfiguration.h"
#include "ui/small_widgets/LanguageCombo.h"

#include <QComboBox>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class TvMazeConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit TvMazeConfigurationView(TvMazeConfiguration& settings);
    ~TvMazeConfigurationView() override = default;

private:
    TvMazeConfiguration& m_settings;
    LanguageCombo* m_languageBox{nullptr};
};

} // namespace scraper
} // namespace mediaelch
