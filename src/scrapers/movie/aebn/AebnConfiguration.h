#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperConfiguration.h"
#include "settings/Settings.h"
#include "utils/Meta.h"

namespace mediaelch {
namespace scraper {

class AebnConfiguration : public QObject, public ScraperConfiguration
{
    Q_OBJECT
public:
    explicit AebnConfiguration(Settings& settings, QObject* parent = nullptr);
    virtual ~AebnConfiguration() = default;

    void init() override;

    ELCH_NODISCARD static QVector<Locale> supportedLanguages();
    ELCH_NODISCARD static QString defaultGenre();

public:
    ELCH_NODISCARD Locale language() override;
    void setLanguage(const Locale& value) override;
    ELCH_NODISCARD static mediaelch::Locale defaultLocale();

    ELCH_NODISCARD QString genreId();
    void setGenreId(const QString& value);

signals:
    void languageChanged(Locale language);
    void genreIdChanged(QString value);
};

} // namespace scraper
} // namespace mediaelch
