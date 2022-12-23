#pragma once

#include "data/Locale.h"
#include "scrapers/tv_show/ShowIdentifier.h"
#include "scrapers/tv_show/TvScraper.h"

#include <QPointer>
#include <QWidget>


namespace Ui {
class TvShowScrapePreview;
}

class TvShowScrapePreview : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowScrapePreview(QWidget* parent = nullptr);

    void loadPreviewFor(mediaelch::scraper::TvScraper* scraper,
        mediaelch::scraper::ShowIdentifier id,
        mediaelch::Locale locale);
    void clearAndAbortPreview();

signals:

private slots:
    void onScrapeJobFinished(mediaelch::scraper::ShowScrapeJob* scrapeJob);

private:
    void abortCurrentJobs();

private:
    Ui::TvShowScrapePreview* ui = nullptr;

    QPointer<mediaelch::scraper::ShowScrapeJob> m_currentScrapeJob = nullptr;

    QPixmap m_placeholderPoster;
};
