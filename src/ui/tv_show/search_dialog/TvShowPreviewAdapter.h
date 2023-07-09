#pragma once

#include "data/tv_show/TvShow.h"
#include "scrapers/tv_show/ShowScrapeJob.h"
#include "scrapers/tv_show/TvScraper.h"
#include "ui/small_widgets/ScrapePreview.h"
#include "utils/Meta.h"

#include <QPointer>
#include <memory>

namespace mediaelch {

class TvShowPreviewAdapter : public ScrapePreview::JobAdapter
{
public:
    static std::unique_ptr<ScrapePreview::JobAdapter>
    createFor(scraper::TvScraper* scraper, scraper::ShowIdentifier id, Locale locale);

public:
    explicit TvShowPreviewAdapter(scraper::ShowScrapeJob* scrapeJob) : m_scrapeJob{scrapeJob} {}
    ~TvShowPreviewAdapter() override;

    ELCH_NODISCARD worker::Job* scrapeJob() override { return m_scrapeJob.data(); }

    ELCH_NODISCARD QString title() override
    {
        MediaElch_Expects(m_scrapeJob != nullptr);
        return m_scrapeJob->tvShow().title();
    }

    ELCH_NODISCARD QString description() override
    {
        MediaElch_Expects(m_scrapeJob != nullptr);
        return m_scrapeJob->tvShow().overview();
    }

    ELCH_NODISCARD Poster poster() override
    {
        MediaElch_Expects(m_scrapeJob != nullptr);
        if (!m_scrapeJob->tvShow().posters().isEmpty()) {
            return m_scrapeJob->tvShow().posters().first();
        } else {
            return {};
        }
    }

private:
    QPointer<scraper::ShowScrapeJob> m_scrapeJob{nullptr};
};

} // namespace mediaelch
