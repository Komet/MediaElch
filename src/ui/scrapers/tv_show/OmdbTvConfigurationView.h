#pragma once

#include "scrapers/tv_show/omdb/OmdbTvConfiguration.h"

#include <QLineEdit>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class OmdbTvConfigurationView : public QWidget
{
    Q_OBJECT

public:
    explicit OmdbTvConfigurationView(OmdbTvConfiguration& settings);
    ~OmdbTvConfigurationView() override = default;

private:
    OmdbTvConfiguration& m_settings;
    QLineEdit* m_apiKeyEdit{nullptr};
};

} // namespace scraper
} // namespace mediaelch
