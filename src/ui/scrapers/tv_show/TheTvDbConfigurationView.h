#pragma once

#include "scrapers/tv_show/thetvdb/TheTvDbConfiguration.h"
#include "ui/small_widgets/LanguageCombo.h"

#include <QComboBox>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class TheTvDbConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit TheTvDbConfigurationView(TheTvDbConfiguration& settings);
    ~TheTvDbConfigurationView() override = default;

private:
    TheTvDbConfiguration& m_settings;
    LanguageCombo* m_languageBox{nullptr};
};

} // namespace scraper
} // namespace mediaelch
