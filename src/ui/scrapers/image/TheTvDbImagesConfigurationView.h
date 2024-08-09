#pragma once

#include "scrapers/image/TheTvDbImagesConfiguration.h"
#include "ui/small_widgets/LanguageCombo.h"

#include <QWidget>

namespace mediaelch {
namespace scraper {

class TheTvDbImagesConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit TheTvDbImagesConfigurationView(TheTvDbImagesConfiguration& settings);
    ~TheTvDbImagesConfigurationView() override = default;

private:
    TheTvDbImagesConfiguration& m_settings;
    LanguageCombo* m_languageBox{nullptr};
};

} // namespace scraper
} // namespace mediaelch
