#pragma once

#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/aebn/AebnApi.h"

#include <QComboBox>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class AebnConfiguration;

class AEBN : public MovieScraper
{
    Q_OBJECT
public:
    explicit AEBN(AebnConfiguration& settings, QObject* parent = nullptr);
    ~AEBN() override;
    static constexpr const char* ID = "aebn";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD MovieSearchJob* search(MovieSearchJob::Config config) override;
    ELCH_NODISCARD MovieScrapeJob* loadMovie(MovieScrapeJob::Config config) override;

public:
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;

    QSet<MovieScraperInfo> scraperNativelySupports() override;

    void changeLanguage(mediaelch::Locale locale) override;
    QWidget* settingsWidget() override;

private:
    AebnConfiguration& m_settings;
    ScraperMeta m_meta;
    AebnApi m_api;

    QPointer<QWidget> m_widget;
    QComboBox* m_box;
    QComboBox* m_genreBox;
};

} // namespace scraper
} // namespace mediaelch
