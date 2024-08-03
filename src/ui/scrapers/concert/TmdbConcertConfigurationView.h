#pragma once

#include "scrapers/concert/tmdb/TmdbConcertConfiguration.h"
#include "ui/small_widgets/LanguageCombo.h"

#include <QComboBox>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class TmdbConcertConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit TmdbConcertConfigurationView(TmdbConcertConfiguration& settings);
    ~TmdbConcertConfigurationView() override = default;

private:
    TmdbConcertConfiguration& m_settings;
    LanguageCombo* m_languageBox{nullptr};
};

} // namespace scraper
} // namespace mediaelch
