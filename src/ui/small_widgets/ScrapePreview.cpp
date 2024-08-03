#include "ui/small_widgets/ScrapePreview.h"
#include "ui_ScrapePreview.h"

#include "data/Poster.h"

namespace {
static const char* const WAS_ABORTED = "WAS_ABORTED";
}

ScrapePreview::ScrapePreview(QWidget* parent) : QWidget{parent}, ui(new Ui::ScrapePreview)
{
    ui->setupUi(this);
}

void ScrapePreview::load(std::unique_ptr<JobAdapter> jobAdapter)
{
    clearAndAbortPreview();

    ui->poster->startLoadingSpinner(); // indicate that we're loading data

    m_currentAdapter = std::move(jobAdapter);
    connect(
        m_currentAdapter->scrapeJob(), &mediaelch::worker::Job::finished, this, &ScrapePreview::onScrapeJobFinished);
    m_currentAdapter->scrapeJob()->start();
}

void ScrapePreview::clearAndAbortPreview()
{
    ui->lblDescriptionContent->clear();
    ui->poster->clearAndAbortDownload();

    std::unique_ptr<JobAdapter> currentJob = std::move(m_currentAdapter);
    m_currentAdapter = nullptr;
    if (currentJob != nullptr && currentJob->scrapeJob() != nullptr && !currentJob->scrapeJob()->isFinished()) {
        // kill() may return false and not kill the job.
        // In that case, it will finish and call onScrapeJobFinished. To distinguish
        // killed jobs from the current one, store a bool.
        currentJob->scrapeJob()->setProperty(WAS_ABORTED, QVariant::fromValue(true));
        currentJob->scrapeJob()->kill();
    }
}

void ScrapePreview::onScrapeJobFinished(mediaelch::worker::Job* scrapeJob)
{
    auto dls = makeDeleteLaterScope(scrapeJob);
    if (scrapeJob->property(WAS_ABORTED).toBool()) {
        // If it was aborted, do not clear adapter; was done before
        return;
    }
    if (!scrapeJob->hasError()) {
        MediaElch_Expects(m_currentAdapter != nullptr);
        MediaElch_Expects(scrapeJob == m_currentAdapter->scrapeJob());

        ui->lblDescriptionContent->setPlainText(m_currentAdapter->description());
        ui->poster->clearAndAbortDownload();

        const Poster poster = m_currentAdapter->poster();
        const QUrl url = !poster.thumbUrl.isEmpty() ? poster.thumbUrl : poster.originalUrl;
        if (url.isValid()) {
            ui->poster->showImageFrom(url);
        }
    }
    m_currentAdapter = nullptr;
}
