#include "scrapers/movie/custom/CustomMovieScraperConfiguration.h"

#include "scrapers/movie/custom/CustomMovieScraper.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_CUSTOM_MOVIE_SCRAPER(moduleName, "CustomMovieScraper");

} // namespace


namespace mediaelch {
namespace scraper {


CustomMovieScraperConfiguration::CustomMovieScraperConfiguration(Settings& settings, QObject* parent) :
    QObject(parent), ScraperConfiguration(QString(CustomMovieScraper::ID), settings)
{
    Q_UNUSED(KEY_CUSTOM_MOVIE_SCRAPER)
}

void CustomMovieScraperConfiguration::init()
{
    // FIXME
}

Locale CustomMovieScraperConfiguration::language()
{
    return Locale::NoLocale;
}

void CustomMovieScraperConfiguration::setLanguage(const Locale& value)
{
    Q_UNUSED(value)
    // no-op
}

QMap<MovieScraperInfo, QString> CustomMovieScraperConfiguration::detailScraperMap() const
{
    // FIXME
    return QMap<MovieScraperInfo, QString>();
}

void CustomMovieScraperConfiguration::setDetailScraperMap(QMap<MovieScraperInfo, QString> customMovieScraper)
{
    // FIXME
    Q_UNUSED(customMovieScraper)
}

} // namespace scraper
} // namespace mediaelch
