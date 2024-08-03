#pragma once

#include "scrapers/tv_show/fernsehserien_de/FernsehserienDeConfiguration.h"
#include "ui/small_widgets/LanguageCombo.h"

#include <QComboBox>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class FernsehserienDeConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit FernsehserienDeConfigurationView(FernsehserienDeConfiguration& settings);
    ~FernsehserienDeConfigurationView() override = default;

private:
    FernsehserienDeConfiguration& m_settings;
    LanguageCombo* m_languageBox{nullptr};
};

} // namespace scraper
} // namespace mediaelch
