#include "ui/tv_show/search_dialog/TvShowPreviewAdapter.h"

namespace mediaelch {

TvShowPreviewAdapter::~TvShowPreviewAdapter() = default;

std::unique_ptr<ScrapePreview::JobAdapter>
TvShowPreviewAdapter::createFor(scraper::TvScraper* scraper, scraper::ShowIdentifier id, Locale locale)
{
    scraper::ShowScrapeJob::Config config;
    config.identifier = std::move(id);
    config.locale = std::move(locale);
    config.details = {ShowScraperInfo::Title, ShowScraperInfo::Overview, ShowScraperInfo::Poster};

    scraper::ShowScrapeJob* job = scraper->loadShow(config);
    return std::make_unique<TvShowPreviewAdapter>(job);
}

} // namespace mediaelch
