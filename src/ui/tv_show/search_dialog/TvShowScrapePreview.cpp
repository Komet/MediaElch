#include "ui/tv_show/search_dialog/TvShowScrapePreview.h"
#include "ui_TvShowScrapePreview.h"

#include "data/Poster.h"
#include "data/tv_show/TvShow.h"
#include "scrapers/tv_show/ShowScrapeJob.h"

namespace {
static const char* WAS_ABORTED = "WAS_ABORTED";
}

TvShowScrapePreview::TvShowScrapePreview(QWidget* parent) : QWidget{parent}, ui(new Ui::TvShowScrapePreview)
{
    ui->setupUi(this);
}

void TvShowScrapePreview::loadPreviewFor(mediaelch::scraper::TvScraper* scraper,
    mediaelch::scraper::ShowIdentifier id,
    mediaelch::Locale locale)
{
    using namespace mediaelch::scraper;

    clearAndAbortPreview();

    ui->poster->startLoadingSpinner(); // indicate that we're loading data

    ShowScrapeJob::Config config;
    config.identifier = std::move(id);
    config.locale = std::move(locale);
    config.details = {ShowScraperInfo::Title, ShowScraperInfo::Overview, ShowScraperInfo::Poster};

    m_currentScrapeJob = scraper->loadShow(config);
    connect(m_currentScrapeJob, &ShowScrapeJob::loadFinished, this, &TvShowScrapePreview::onScrapeJobFinished);
    m_currentScrapeJob->start();
}

void TvShowScrapePreview::clearAndAbortPreview()
{
    ui->lblDescriptionContent->clear();
    ui->poster->clearAndAbortDownload();

    auto currentScrapeJob = m_currentScrapeJob;
    m_currentScrapeJob = nullptr;
    if (currentScrapeJob != nullptr && !currentScrapeJob->isFinished()) {
        // kill() may return false and not kill the job.
        // In that case, it will finish and call onScrapeJobFinished. To distinguish
        // killed jobs from the current one, store a bool.
        currentScrapeJob->setProperty(WAS_ABORTED, QVariant::fromValue(true));
        currentScrapeJob->kill();
    }
}

void TvShowScrapePreview::onScrapeJobFinished(mediaelch::scraper::ShowScrapeJob* scrapeJob)
{
    auto dls = makeDeleteLaterScope(scrapeJob);
    if (scrapeJob->hasError() || scrapeJob->property(WAS_ABORTED).toBool()) {
        return;
    }

    MediaElch_Expects(scrapeJob == m_currentScrapeJob);

    ui->lblDescriptionContent->setPlainText(scrapeJob->tvShow().overview());
    ui->poster->clearAndAbortDownload();

    if (!scrapeJob->tvShow().posters().isEmpty()) {
        const Poster poster = scrapeJob->tvShow().posters().first();
        const QUrl url = !poster.thumbUrl.isEmpty() ? poster.thumbUrl : poster.originalUrl;
        ui->poster->showImageFrom(url);
    }
}
