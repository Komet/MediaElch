#pragma once

#include "scrapers/tv_show/imdb/ImdbTvConfiguration.h"
#include "ui/small_widgets/LanguageCombo.h"

#include <QComboBox>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class ImdbTvConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit ImdbTvConfigurationView(ImdbTvConfiguration& settings);
    ~ImdbTvConfigurationView() override = default;

private:
    ImdbTvConfiguration& m_settings;
    LanguageCombo* m_languageBox{nullptr};
};

} // namespace scraper
} // namespace mediaelch
