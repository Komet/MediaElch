#pragma once

#include "scrapers/movie/omdb/OmdbMovieConfiguration.h"

#include <QLineEdit>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class OmdbMovieConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit OmdbMovieConfigurationView(OmdbMovieConfiguration& settings);
    ~OmdbMovieConfigurationView() override = default;

private:
    OmdbMovieConfiguration& m_settings;
    QLineEdit* m_apiKeyEdit{nullptr};
};

} // namespace scraper
} // namespace mediaelch
