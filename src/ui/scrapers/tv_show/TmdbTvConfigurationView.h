#pragma once

#include "scrapers/tv_show/tmdb/TmdbTvConfiguration.h"
#include "ui/small_widgets/LanguageCombo.h"

#include <QComboBox>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class TmdbTvConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit TmdbTvConfigurationView(TmdbTvConfiguration& settings);
    ~TmdbTvConfigurationView() override = default;

private:
    TmdbTvConfiguration& m_settings;
    LanguageCombo* m_languageBox{nullptr};
};

} // namespace scraper
} // namespace mediaelch
