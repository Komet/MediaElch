#pragma once

#include "scrapers/movie/tmdb/TmdbMovieConfiguration.h"
#include "ui/small_widgets/LanguageCombo.h"

#include <QWidget>

namespace mediaelch {
namespace scraper {

class TmdbMovieConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit TmdbMovieConfigurationView(TmdbMovieConfiguration& settings, QWidget* parent = nullptr);
    ~TmdbMovieConfigurationView() override = default;

private:
    TmdbMovieConfiguration& m_settings;
    LanguageCombo* m_languageBox{nullptr};
};

} // namespace scraper
} // namespace mediaelch
