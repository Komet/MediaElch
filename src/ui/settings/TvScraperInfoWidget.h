#pragma once

#include "scrapers/tv_show/TvScraper.h"

#include <QWidget>

namespace Ui {
class TvScraperInfoWidget;
}

class TvScraperInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TvScraperInfoWidget(mediaelch::scraper::TvScraper& scraper, QWidget* parent = nullptr);
    ~TvScraperInfoWidget() override;

private:
    void setupScraperDetails();

private:
    Ui::TvScraperInfoWidget* ui = nullptr;
    mediaelch::scraper::TvScraper& m_scraper;
};
