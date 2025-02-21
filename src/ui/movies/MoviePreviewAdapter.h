#pragma once

#include "data/movie/Movie.h"
#include "scrapers/movie/MovieScrapeJob.h"
#include "scrapers/movie/MovieScraper.h"
#include "ui/small_widgets/ScrapePreview.h"
#include "utils/Meta.h"

#include <QPointer>
#include <memory>

namespace mediaelch {

class MoviePreviewAdapter : public ScrapePreview::JobAdapter
{
public:
    static std::unique_ptr<ScrapePreview::JobAdapter>
    createFor(scraper::MovieScraper* scraper, scraper::MovieIdentifier id, Locale locale);

public:
    explicit MoviePreviewAdapter(scraper::MovieScrapeJob* scrapeJob) : m_scrapeJob{scrapeJob} {}
    ~MoviePreviewAdapter() override;

    ELCH_NODISCARD worker::Job* scrapeJob() override { return m_scrapeJob.data(); }

    ELCH_NODISCARD QString title() override
    {
        MediaElch_Expects(m_scrapeJob != nullptr);
        return m_scrapeJob->movie().title();
    }

    ELCH_NODISCARD QString description() override
    {
        MediaElch_Expects(m_scrapeJob != nullptr);
        return m_scrapeJob->movie().overview();
    }

    ELCH_NODISCARD Poster poster() override
    {
        MediaElch_Expects(m_scrapeJob != nullptr);
        if (!m_scrapeJob->movie().images().posters().isEmpty()) {
            return m_scrapeJob->movie().images().posters().first();
        } else {
            return {};
        }
    }

private:
    QPointer<scraper::MovieScrapeJob> m_scrapeJob{nullptr};
};

} // namespace mediaelch
